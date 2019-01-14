/*
* Copyright (c) 2017-2018  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <strstream>
#include <iostream>
#include <cstring>
#include "../include/celex4/celex4.h"
#include "../frontpanel/frontpanel.h"
#include "../configproc/hhsequencemgr.h"
#include "../base/xbase.h"
#include "../include/celex4/celex4processeddata.h"
#include "dataprocessthread.h"
#include "datareaderthread.h"
#include "fpgadatareader.h"
#include "fpgadataprocessor.h"
#include "datarecorder.h"

// This is the constructor of a class that has been exported.
// see CelexSensorDLL.h for the class definition
CeleX4::CeleX4()
	: m_uiFullPicFrameTime(20)
	, m_uiEventFrameTime(60)
	, m_uiFEFrameTime(60)
	, m_uiOverlapTime(0)
	, m_uiClockRate(25)
	, m_lPlaybackFileSize(0)
	, m_bReadDataByAPICore(false)
	, m_bAutoAdjustBrightness(false)
	, m_lPreT(0)
	, m_fTSum(0)
	, m_lCount(0)
	, m_fDenoiseCount(0)
	, m_fCompressCount(0)
	, m_iDelta(100)
	, m_fTao(0)
    , m_bRecordingVideo(false)
	, m_pReadBuffer(NULL)
{
	m_pFrontPanel = FrontPanel::getInstance();

	m_pSequenceMgr = new HHSequenceMgr;
	m_pSequenceMgr->parseCommandList();
	m_pSequenceMgr->parseSequenceList();
	m_pSequenceMgr->parseSliderList();

	parseSliderSequence();

	m_pDataRecorder = new DataRecorder;

	//create data process thread
	m_pDataProcessThread = new DataProcessThread;
	m_pDataProcessThread->start();

	m_pDataReaderThread = new DataReaderThread(this);
}

CeleX4::~CeleX4()
{
	if (m_ifstreamPlayback.is_open())
		m_ifstreamPlayback.close();

	if (m_pSequenceMgr)
	{
		delete m_pSequenceMgr;
	}
	if (m_pDataProcessThread)
	{
		m_pDataProcessThread->terminate();
		delete m_pDataProcessThread;
	}
	if (m_pFrontPanel)
	{
		m_pFrontPanel->uninitializeFPGA();
		delete m_pFrontPanel;
	}
	if (m_pReadBuffer)
	{
		delete[] m_pReadBuffer;
		m_pReadBuffer = NULL;
	}
}

CeleX4::ErrorCode CeleX4::openSensor(string str)
{
	m_pFrontPanel->initializeFPGA("top.bit");
	if (isSensorReady())
	{
		if (!powerUp())
			return CeleX4::PowerUpFailed;
		if (!configureSettings())
			return CeleX4::ConfigureFailed;

		//cout << "****************************************" << endl;
		//excuteCommand("SetMode Total Time");
		setFEFrameTime(60);
		setFullPicFrameTime(40);

		if (m_bReadDataByAPICore)
		{
			m_pDataReaderThread->startReadData(true);
			m_pDataReaderThread->start();
		}
	}
	else
	{
		return CeleX4::InitializeFPGAFailed;
	}
	return CeleX4::NoError;
}

// Execute Sensor Power-up Sequence
bool CeleX4::powerUp()
{
	bool bSuccess = false;
	std::string sPowerUp = "Power Up";
	if (HHSequence* pUpSeq = m_pSequenceMgr->getSequenceByName(sPowerUp))
	{
		if (!pUpSeq->fire())
			cout << "Power Up failed." << endl;
		else
			bSuccess = true;
	}
	else
	{
		cout << sPowerUp << ": Sequence not defined." << endl;
	}
	return bSuccess;
}

bool CeleX4::configureSettings()
{
	bool bOk = false;
	std::string settingNames;
	if (m_pFrontPanel->isReady())
	{
		bOk = true;
		int index = 0;
		for (vector<string>::iterator itr = m_vecAdvancedNames.begin(); itr != m_vecAdvancedNames.end(); itr++)
		{
			bool ret = setAdvancedBias(m_vecAdvancedNames.at(index));
			if (!ret)
				settingNames += (" " + (*itr));
			bOk = (bOk && ret);
			++index;
		}
	}
	if (bOk)
		cout << "Configure Advanced Settings Successfully!" << endl;
	else
		cout << "Configure Advanced Settings Failed @" << settingNames << endl;
	return bOk;
}

bool CeleX4::isSensorReady()
{
	return m_pFrontPanel->isReady();
}

bool CeleX4::isSdramFull()
{
	uint32_t sdramFull;
	FrontPanel::getInstance()->wireOut(0x20, 0x0001, &sdramFull);
	if (sdramFull > 0)
	{
		cout << "---- SDRAM is full! -----" << endl;
		return true;
	}
	return false;
}

void CeleX4::pipeOutFPGAData()
{
	if (!isSensorReady())
	{
		return;
	}
	uint32_t pageCount;
	m_pFrontPanel->wireOut(0x21, 0x1FFFFF, &pageCount);
	if (pageCount > MAX_PAGE_COUNT)
		pageCount = MAX_PAGE_COUNT;
	m_uiPageCount = pageCount;
	//cout << "-------------- pageCount = " << pageCount << endl;
	int blockSize = 128;
	long length = (long)(pageCount * blockSize);
	if (NULL == m_pReadBuffer)
		m_pReadBuffer = new unsigned char[128 * MAX_PAGE_COUNT];
	//Return the number of bytes read or ErrorCode (<0) if the read failed. 
	long dataLen = m_pFrontPanel->blockPipeOut(0xa0, blockSize, length, m_pReadBuffer);
	if (dataLen > 0)
	{
		//record sensor data
		if (m_pDataRecorder->isRecording())
		{
			m_pDataRecorder->writeData(m_pReadBuffer, dataLen);
		}
		m_pDataProcessThread->addData(m_pReadBuffer, dataLen);
	}
	else if (dataLen < 0) //read failed
	{
		switch (dataLen)
		{
		case okCFrontPanel::InvalidBlockSize:
			cout << "Block Size Not Supported" << endl;
			break;

		case okCFrontPanel::UnsupportedFeature:
			cout << "Unsupported Feature" << endl;
			break;

		default:
			cout << "Transfer Failed with error: " << dataLen << endl;
			break;
		}
		cout << "pageCount = " << pageCount << ", blockSize = " << blockSize << ", length = " << length << endl;
	}
	if (m_bAutoAdjustBrightness)
		autoAdjustBrightness();
}

long CeleX4::getFPGADataSize()
{
	if (!isSensorReady())
	{
		return -1;
	}
	uint32_t pageCount;
	m_pFrontPanel->wireOut(0x21, 0x1FFFFF, &pageCount);
	m_uiPageCount = pageCount;
	//cout << "----------- pageCount = " << pageCount << endl; 
	return 128 * pageCount;
}

long CeleX4::readDataFromFPGA(long length, unsigned char *data)
{
	if (!data)
		return -1;
	if (!isSensorReady())
	{
		return -1;
	}
	uint32_t pageCount;
	m_pFrontPanel->wireOut(0x21, 0x1FFFFF, &pageCount);
	m_uiPageCount = pageCount;
	//cout << "----------- pageCount = " << pageCount << endl;
	int blockSize = 128;
	//Return the number of bytes read or ErrorCode (<0) if the read failed. 
	long dataLen = m_pFrontPanel->blockPipeOut(0xa0, blockSize, length, data);
	if (dataLen > 0)
	{
		return dataLen;
	}
	else if (dataLen < 0) //read failed
	{
		switch (dataLen)
		{
		case okCFrontPanel::InvalidBlockSize:
			cout << "Block Size Not Supported: " << dataLen << endl;
			break;

		case okCFrontPanel::UnsupportedFeature:
			cout << "Unsupported Feature: " << dataLen << endl;
			break;

		default:
			cout << "Transfer Failed with error: " << dataLen << endl;
			break;
		}
	}
	return -1;
}

unsigned char *CeleX4::getFullPicBuffer()
{
	CeleX4ProcessedData* pSensorData = getSensorDataObject();
	if (!pSensorData)
	{
		return NULL;
	}
	return pSensorData->getFullPicBuffer();
}

unsigned char *CeleX4::getEventPicBuffer(emEventPicMode mode)
{
	CeleX4ProcessedData* pSensorData = getSensorDataObject();
	if (!pSensorData)
	{
		return NULL;
	}
	if (EventAccumulatedPic == mode ||
		EventBinaryPic == mode ||
		EventGrayPic == mode ||
		EventSuperimposedPic == mode ||
		EventDenoisedBinaryPic == mode ||
		EventDenoisedGrayPic == mode ||
		EventCountPic == mode /*||
		EventDenoisedByTimeBinaryPic == mode ||
		EventDenoisedByTimeGrayPic == mode*/)
	{
		return pSensorData->getEventPicBuffer(mode);
	}
	else
	{
		return NULL;
	}
}

//golbal reset length
//address:0x05, mask: 31:12
void CeleX4::setResetLength(uint32_t value)
{
	//--- excuteCommand("SetMode Global Reset Length"); ---
	value = value << 12;
	FrontPanel::getInstance()->wireIn(0x05, value, 0xFFFFF000);
	FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x05 << "; Value: " << value << "; Mask: " << 0xFFFFF000 << endl;
}

void CeleX4::clearData()
{
	m_pDataProcessThread->clearData();
}

// Execute Sensor "Event Mode"/"Full Picture" Sequence
void CeleX4::setSensorMode(emSensorMode mode)
{
	if (!m_pFrontPanel->isReady())
		return;

	if (mode == EventMode)
	{
		excuteCommand("Full Picture");
		m_pDataProcessThread->setSensorMode(mode);
	}
	else if (mode == FullPictureMode)
	{
		excuteCommand("Event Mode");
		m_pDataProcessThread->setSensorMode(mode);
	}
	else //AC or AB or ABC Mode
	{
		excuteCommand("Optical Mode");
		m_pDataProcessThread->setSensorMode(mode);
	}
}

emSensorMode CeleX4::getSensorMode()
{
	return m_pDataProcessThread->getDataProcessor()->getSensorMode();
}

bool CeleX4::setFpnFile(const string &fpnFile)
{
	return m_pDataProcessThread->getDataProcessor()->setFpnFile(fpnFile);
}

void CeleX4::generateFPN(std::string fpnFile)
{
	m_pDataProcessThread->getDataProcessor()->generateFPN(fpnFile);
}

void CeleX4::resetFPGA()
{
	excuteCommand("Reset-Dereset FPGA");
}

void CeleX4::resetSensorAndFPGA()
{
	excuteCommand("Reset-Dereset All");
}

void CeleX4::enableADC(bool enable)
{
	if (enable)
		excuteCommand("ADC Disable");
	else
		excuteCommand("ADC Enalbe");
}

void CeleX4::setContrast(uint32_t value)
{
	uint32_t uiREF_PLUS = 512 + value / 2;
	uint32_t uiREF_MINUS = 512 - value / 2;
	uint32_t uiREF_PLUS_H = uiREF_PLUS + value / 16;
	uint32_t uiREF_MINUS_H = uiREF_MINUS - value / 16;

	m_mapSliderNameValue["REF+"] = uiREF_PLUS;
	m_mapSliderNameValue["REF-"] = uiREF_MINUS;
	m_mapSliderNameValue["REF+H"] = uiREF_PLUS_H;
	m_mapSliderNameValue["REF-H"] = uiREF_MINUS_H;

	setAdvancedBias("REF+", uiREF_PLUS);
	setAdvancedBias("REF-", uiREF_MINUS);
	setAdvancedBias("REF+H", uiREF_PLUS_H);
	setAdvancedBias("REF-H", uiREF_MINUS_H);
}

uint32_t CeleX4::getContrast()
{
	return (m_mapSliderNameValue["REF+"] - 512) * 2;
}

void CeleX4::setBrightness(uint32_t value)
{
	m_mapSliderNameValue["CDS_DC"] = value;
	setAdvancedBias("CDS_DC", value);
}

uint32_t CeleX4::getBrightness()
{
	return m_mapSliderNameValue["CDS_DC"];
}

void CeleX4::setThreshold(uint32_t value)
{
	unsigned uiEVT_DC = m_mapSliderNameValue["EVT_DC"];
	uint32_t uiEVT_VL = uiEVT_DC - value;
	uint32_t uiEVT_VH = uiEVT_DC + value;

	m_mapSliderNameValue["EVT_VL"] = uiEVT_VL;
	m_mapSliderNameValue["EVT_VH"] = uiEVT_VH;

	setAdvancedBias("EVT_VL", uiEVT_VL);
	setAdvancedBias("EVT_VH", uiEVT_VH);
}

uint32_t CeleX4::getThreshold()
{
	return m_mapSliderNameValue["EVT_VH"] - m_mapSliderNameValue["EVT_DC"];
}

void CeleX4::trigFullPic()
{
	excuteCommand("Force Fire");
}

void CeleX4::setLowerADC(uint32_t value)
{
	m_pDataProcessThread->getDataProcessor()->setLowerADC(value);
}
uint32_t CeleX4::getLowerADC()
{
	return m_pDataProcessThread->getDataProcessor()->getLowerADC();
}

void CeleX4::setUpperADC(uint32_t value)
{
	m_pDataProcessThread->getDataProcessor()->setUpperADC(value);
}
uint32_t CeleX4::getUpperADC()
{
	return m_pDataProcessThread->getDataProcessor()->getUpperADC();
}

void CeleX4::startRecording(std::string filePath)
{
	m_pDataRecorder->startRecording(filePath);
}

void CeleX4::stopRecording()
{
	m_pDataRecorder->stopRecording(m_uiClockRate, m_pDataProcessThread->getDataProcessor()->getSensorMode());
}

CeleX4::TimeInfo CeleX4::getRecordedTime()
{
	return m_stTimeRecorded;
}

/*
@param filePath: Path of the output video file.
@param fullPicName: FullPic data name of the output video file.
@param eventName: Event data name of the output video file.
@param fourcc: 4-character code of codec used to compress the frames, opencv parameter
@param fps: Framerate of the created video stream, opencv parameter
*/
void CeleX4::startRecordingVideo(std::string filePath, std::string fullPicName, std::string eventName, int fourcc, double fps)
{
	cout << "OpenCVVideoWriter::startRecordingVideo" << endl;
	std::string fileFullPic = filePath + fullPicName;
	cout << fileFullPic << endl;
	std::string fileEvent = filePath + eventName;
	cout << fileEvent << endl;
	//g_cvVideoWriter.open(file, CV_FOURCC('X', 'V', 'I', 'D'), 20.0, cv::Size(768, 640), false);
	if (FullPictureMode == getSensorMode() || FullPic_Event_Mode == getSensorMode())
		g_cvVideoWriterFullPic.open(fileFullPic, fourcc, fps, cv::Size(768, 640), false);
	if (EventMode == getSensorMode() || FullPic_Event_Mode == getSensorMode())
		g_cvVideoWriterEvent.open(fileEvent, fourcc, fps, cv::Size(768, 640), false);
	m_bRecordingVideo = true;
}

void CeleX4::stopRecordingVideo()
{
	cout << "OpenCVVideoWriter::stopRecordingVideo" << endl;
	if (g_cvVideoWriterFullPic.isOpened())
		g_cvVideoWriterFullPic.release();
	if (g_cvVideoWriterEvent.isOpened())
		g_cvVideoWriterEvent.release();

	m_bRecordingVideo = false;
}

bool CeleX4::readPlayBackData(long length)
{
	//cout << __FUNCTION__ << endl;
	bool eof = false;
	int maxLen = 128 * MAX_PAGE_COUNT;
	int lenToRead = length > maxLen ? maxLen : length;

	if (NULL == m_pReadBuffer)
		m_pReadBuffer = new unsigned char[maxLen];

	while (true && m_pDataProcessThread->queueSize() < 1000000)
	{
		m_ifstreamPlayback.read((char*)m_pReadBuffer, lenToRead);

		int dataLen = m_ifstreamPlayback.gcount();
		if (dataLen > 0)
			m_pDataProcessThread->addData(m_pReadBuffer, dataLen);

		if (m_ifstreamPlayback.eof())
		{
			eof = true;
			m_pDataProcessThread->setPlaybackState(BinReadFinished);
			//m_ifstreamPlayback.close();
			cout << "Read Playback file Finished!" << endl;
			break;
		}
	}
	return eof;
}

bool CeleX4::openPlaybackFile(string filePath)
{
	if (m_ifstreamPlayback.is_open())
		m_ifstreamPlayback.close();

	m_ifstreamPlayback.open(filePath.c_str(), std::ios::binary);
	if (!m_ifstreamPlayback.good())
	{
		cout << "Can't Open File: " << filePath.c_str();
		return false;
	}

	// read header
	char header[8];
	m_ifstreamPlayback.read(header, 8);
	if ((header[0] != 0 || header[1] != 0 || header[2] != 0) && header[3] == 123)
	{
		m_stTimeRecorded.second = header[0];
		m_stTimeRecorded.minute = header[1];
		m_stTimeRecorded.hour = header[2];
	}
	else
	{
		m_ifstreamPlayback.seekg(0, ios::end);
		m_lPlaybackFileSize = m_ifstreamPlayback.tellg();
		long aa = 1000000;
		long time = (m_lPlaybackFileSize / aa) * 3;

		m_ifstreamPlayback.seekg(0, ios::beg);
		m_stTimeRecorded.second = (time % 3600) % 60;
		m_stTimeRecorded.minute = (time % 3600) / 60;
		m_stTimeRecorded.hour = time / 3600;
	}
	if (header[7] >= 0)
		m_pDataProcessThread->setSensorMode((emSensorMode)header[7]);
	else
		m_pDataProcessThread->setSensorMode(EventMode);

	if (header[6] > 1)
	{
		m_pDataProcessThread->getDataProcessor()->setFPGATimeCycle(131072);
		m_uiClockRate = header[6];
		m_pDataProcessThread->getDataProcessor()->setClockRate(m_uiClockRate);
	}
	else
	{
		m_pDataProcessThread->getDataProcessor()->setFPGATimeCycle(262144);
		m_uiClockRate = 25;
		m_pDataProcessThread->getDataProcessor()->setClockRate(m_uiClockRate);
	}
	cout << "sensor mode = " << (int)header[7] << ", clock = " << (int)header[6] << endl;
	return true;
}

void CeleX4::play()
{
	//if (!m_pDataProcessThread->isRunning())
	m_pDataProcessThread->resume();
}

void CeleX4::pause()
{
	if (m_pDataProcessThread->isRunning())
		m_pDataProcessThread->suspend();
}

long CeleX4::getPlaybackFileSize()
{
	return m_lPlaybackFileSize;
}

bool CeleX4::setPlayBackOffset(long offset)
{
	m_pDataProcessThread->clearData();
	//if (m_ifstreamPlayback.eof())
	{
		m_ifstreamPlayback.clear();
	}
	m_ifstreamPlayback.seekg(offset, ios::beg);

	return true;
}

void CeleX4::saveSelectedBinFile(string filePath, long fromPos, long toPos, int hour, int minute, int second)
{
	if (fromPos == toPos)
		return;

	std::ofstream ofstream;
	ofstream.open(filePath.c_str(), std::ios::binary);
	if (!ofstream.is_open())
	{
		cout << "Can't open saveSelectedBinFile: " << filePath.c_str() << endl;
		return;
	}
	// write a header
	char header[8];
	header[0] = second;
	header[1] = minute;
	header[2] = hour;
	header[3] = 123;
	header[4] = 0;
	header[5] = 0;
	header[6] = m_uiClockRate;
	header[7] = m_pDataProcessThread->getDataProcessor()->getSensorMode(); // !

	ofstream.write(header, sizeof(header));

	long lenToRead = toPos - fromPos;
	char* data = new char[lenToRead];

	m_ifstreamPlayback.clear();
	m_ifstreamPlayback.seekg(fromPos, ios::beg);
	m_ifstreamPlayback.read((char*)data, lenToRead);
	ofstream.write(data, lenToRead);
	ofstream.close();
}

PlaybackState CeleX4::getPlaybackState()
{
	return m_pDataProcessThread->getPlaybackState();
}

void CeleX4::setPlaybackState(PlaybackState state)
{
	m_pDataProcessThread->setPlaybackState(state);
}

//In FullPic and FullPic_Event mode, time block is calculated by FPGA, so it need be set the FGPA
void CeleX4::setFullPicFrameTime(uint32_t msec)
{
	cout << "API: setFullPicFrameTime " << msec << " ms" << endl;
	uint32_t value = msec * 25000;
	//--- excuteCommand("SetMode Total Time"); ---
	FrontPanel::getInstance()->wireIn(0x02, value, 0x00FFFFFF);
	FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0x00FFFFFF << endl;

	m_uiFullPicFrameTime = msec;
}

uint32_t CeleX4::getFullPicFrameTime()
{
	return m_uiFullPicFrameTime;
}

//In Event mode, time block is calculated by software, so it needn't be set the FGPA
void CeleX4::setEventFrameTime(uint32_t msec)
{
	cout << "API: setEventFrameTime " << msec << " ms" << endl;
	m_pDataProcessThread->getDataProcessor()->setTimeSlice(msec);
	m_uiEventFrameTime = msec;
}

uint32_t CeleX4::getEventFrameTime()
{
	return m_uiEventFrameTime;
}

//The usrs hope to set the time block to 1s in FE mode, but there aren't enough bits, 
//in order to solve this problem, FPGA multiplys the msec by 4. 
void CeleX4::setFEFrameTime(uint32_t msec)
{
	cout << " API: setFEFrameTime " << msec << " ms" << endl;
	uint32_t value = msec * 6250; //6250 = 25000/4 
	//--- excuteCommand("SetMode Total Time"); ---
	FrontPanel::getInstance()->wireIn(0x02, value, 0x00FFFFFF);
	FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0x00FFFFFF << endl;

	m_pDataProcessThread->getDataProcessor()->setFEFrameTime(msec);
	m_uiFEFrameTime = msec;
}

uint32_t CeleX4::getFEFrameTime()
{
	return m_uiFEFrameTime;
}

void CeleX4::setFrameLengthRange(float startRatio, float endRatio)
{
	m_pDataProcessThread->getDataProcessor()->setFrameLengthRange(startRatio, endRatio);
}

void CeleX4::setOverlapTime(uint32_t msec)
{
	m_uiOverlapTime = msec;
	m_pDataProcessThread->getDataProcessor()->setOverlapTime(msec);
}

uint32_t CeleX4::getOverlapTime()
{
	return m_uiOverlapTime;
}

CeleX4ProcessedData *CeleX4::getSensorDataObject()
{
	return m_pDataProcessThread->getDataProcessor()->getSensorDataObject();
}

CX4SensorDataServer* CeleX4::getSensorDataServer()
{
	return m_pDataProcessThread->getDataProcessor()->getSensorDataServer();
}

std::vector<CeleX4::ControlSliderInfo> CeleX4::getSensorControlList()
{
	return m_vecSensorControlList;
}

void CeleX4::pauseThread(bool pause1)
{
	if (pause1)
		pause();
	else
		play();
}

// init sliders
void CeleX4::parseSliderSequence()
{
	cout << endl << "***** " << __FUNCTION__ << " *****" << endl;
	std::vector<std::string> sliderNames = m_pSequenceMgr->getAllSliderNames();
	for (vector<string>::iterator itr = sliderNames.begin(); itr != sliderNames.end(); itr++)
	{
		std::string name = *itr;
		HHSequenceSlider* pSliderSeq = m_pSequenceMgr->getSliderByName(name);

		if (!pSliderSeq)
			continue;
		// show or not
		if (!pSliderSeq->isShown())
			continue;
		//save all the control class
		ControlSliderInfo sliderInfo;
		sliderInfo.min = pSliderSeq->getMin();
		sliderInfo.max = pSliderSeq->getMax();
		sliderInfo.value = pSliderSeq->getValue();
		sliderInfo.step = pSliderSeq->getStep();
		sliderInfo.name = pSliderSeq->name();
		sliderInfo.bAdvanced = pSliderSeq->isAdvanced();
		m_vecSensorControlList.push_back(sliderInfo);
		// advanced
		if (pSliderSeq->isAdvanced())
		{
			m_vecAdvancedNames.push_back(name);
			if (HHSequenceSlider* pSliderSeq = m_pSequenceMgr->getSliderByName(name))
			{
				uint32_t initial = pSliderSeq->getValue();
				// keep the initial values
				m_mapSliderNameValue[name] = initial;
				//cout << "m_mapSliderNameValue: " << name << "  " << initial << endl;
			}

			cout << "m_vecAdvancedNames: " << name << endl;
			continue;
		}
		uint32_t initial = pSliderSeq->getValue();
		// keep the initial values
		m_mapSliderNameValue[name] = initial;
		//cout << "m_mapSliderNameValue: " << name << "  " << initial << endl;
	}
}

bool CeleX4::excuteCommand(string strCommand)
{
	bool bSuccess = false;
	if (HHSequence* pUpSeq = m_pSequenceMgr->getSequenceByName(strCommand))
	{
		if (!pUpSeq->fire())
			cout << "excute command failed." << endl;
		else
			bSuccess = true;
	}
	else
	{
		cout << strCommand << ": Sequence not defined." << endl;
	}
	return bSuccess;
}

bool CeleX4::setAdvancedBias(std::string strBiasName)
{
	bool result = false;
	if (HHSequenceSlider* pSeqSlider = m_pSequenceMgr->getSliderByName(strBiasName))
	{
		uint32_t arg = m_mapSliderNameValue[strBiasName];
		result = pSeqSlider->fireWithArg(arg);
		cout << "Advanced Settings loaded: " << strBiasName << "  " << arg << endl;
	}
	return result;
}

bool CeleX4::setAdvancedBias(std::string strBiasName, int value)
{
	bool result = false;
	if (HHSequenceSlider* pSeqSlider = m_pSequenceMgr->getSliderByName(strBiasName))
	{
		result = pSeqSlider->fireWithArg(value);
		cout << "Advanced Settings loaded: " << strBiasName << "  " << value << endl;
	}
	return result;
}

void CeleX4::autoAdjustBrightness()
{
	//cout << "Current: " << m_pSensorData->getMeanIntensity() << ", Lat: " << m_iLastMeanIntensity << endl;
	int meanIntensity = getMeanIntensity();
	if (meanIntensity < 110 || meanIntensity > 140)
	{
		int brightness = getBrightness();
		if (meanIntensity > 220)
			brightness -= 10;
		else if (meanIntensity > 200)
			brightness -= 5;
		else if (meanIntensity > 180)
			brightness -= 2;
		else if (meanIntensity > 150)
			brightness -= 1;
		else if (meanIntensity > 140)
			brightness -= 1;
		else if (meanIntensity < 50)
			brightness += 10;
		else if (meanIntensity < 80)
			brightness += 5;
		else if (meanIntensity < 110)
			brightness += 1;

		if (meanIntensity > 1000 ||
			meanIntensity < 10)
		{
			brightness = 250;
		}
		setBrightness(brightness);
		//m_pWindow->setSliderValue("Brightness", brightness);
	}
}

uint32_t CeleX4::getClockRate()
{
	return m_uiClockRate;
}
//value * 2 = clock = 100 * M / D
void CeleX4::setClockRate(uint32_t value)
{
	cout << "************* API: setClockRate " << value << " MHz" << endl;
	m_uiClockRate = value;
	if (value > 50)
		value = 50;
	uint32_t valueM, valueD = 0x00630000;
	valueM = (value * 2 - 1) << 24;
	FrontPanel::getInstance()->wireIn(0x03, valueM, 0xFF000000); //M: [31:24]
	FrontPanel::getInstance()->wireIn(0x03, valueD, 0x00FF0000); //D: [23:16]

	FrontPanel::getInstance()->wireIn(0x03, 0, 0x00008000); //Apply OFF [15]
	FrontPanel::getInstance()->wait(1);
	FrontPanel::getInstance()->wireIn(0x03, 0x00008000, 0x00008000); //Apply ON  [15]

	m_pDataProcessThread->getDataProcessor()->setClockRate(value);
}

void CeleX4::enableOpticalFlow(bool enable)
{
	m_pDataProcessThread->getDataProcessor()->enableMultiSlice(enable);
	m_bOpticalFlowEnable = enable;
}

bool CeleX4::isOpticalFlowEnabled()
{
	return m_pDataProcessThread->getDataProcessor()->isMultiSliceEnabled();
}

void CeleX4::setOpticalFlowLatencyTime(uint32_t msec)
{
	cout << "************* API: setOpticalFlowLatencyTime " << msec << " ms" << endl;
	m_pDataProcessThread->getDataProcessor()->setMultiSliceTime(msec);
}

void CeleX4::setOpticalFlowSliceCount(uint32_t count)
{
	cout << "************* API: setOpticalFlowSliceCount " << count << endl;
	m_pDataProcessThread->getDataProcessor()->setMultiSliceCount(count);
}

unsigned char* CeleX4::getOpticalFlowPicBuffer()
{
	if (isOpticalFlowEnabled())
	{
		CeleX4ProcessedData* pSensorData = getSensorDataObject();
		if (!pSensorData)
		{
			return NULL;
		}
		return pSensorData->getOpticalFlowPicBuffer();
	}
	return NULL;
}

unsigned char* CeleX4::getOpticalFlowDirectionPicBuffer()
{
	if (isOpticalFlowEnabled())
	{
		CeleX4ProcessedData* pSensorData = getSensorDataObject();
		if (!pSensorData)
		{
			return NULL;
		}
		return pSensorData->getOpticalFlowDirectionPicBuffer();
	}
	return NULL;
}

unsigned char* CeleX4::getOpticalFlowSpeedPicBuffer()
{
	if (isOpticalFlowEnabled())
	{
		CeleX4ProcessedData* pSensorData = getSensorDataObject();
		if (!pSensorData)
		{
			return NULL;
		}
		return pSensorData->getOpticalFlowSpeedPicBuffer();
	}
	return NULL;
}

uint32_t CeleX4::getPageCount()
{
	return m_uiPageCount;
}

uint32_t CeleX4::getMeanIntensity()
{
	return m_pDataProcessThread->getDataProcessor()->getMeanIntensity();
}

unsigned long CeleX4::getSpecialEventCount()
{
	return m_pDataProcessThread->getDataProcessor()->getSpecialEventCount();
}

void CeleX4::setSpecialEventCount(unsigned long count)
{
	m_pDataProcessThread->getDataProcessor()->setSpecialEventCount(count);
}

std::vector<int> CeleX4::getDataLengthPerSpecial()
{
	return m_pDataProcessThread->getDataProcessor()->getDataLengthPerSpecial();
}

std::vector<unsigned long> CeleX4::getEventCountListPerSpecial()
{
	return m_pDataProcessThread->getDataProcessor()->getEventCountListPerSpecial();
}

BinFileAttributes CeleX4::getAttributes(std::string binFile)	
{
	return m_pDataProcessThread->getDataProcessor()->getBinFileAttributes(binFile);
}

void CeleX4::setTimeScale(float scale)
{
	m_pDataProcessThread->getDataProcessor()->setTimeScale(scale);
}

void CeleX4::setEventCountStepSize(uint32_t size)
{
	m_pDataProcessThread->getDataProcessor()->setEventCountStepSize(size);
}

bool CeleX4::getEventDataVector(std::vector<EventData> &vector)
{
	/*if (m_pDataProcessThread->getDataProcessor()->getSensorDataObject()->getEventDataVector().size() > 0)
	{
		vector.swap(m_pDataProcessThread->getDataProcessor()->getSensorDataObject()->getEventDataVector());
		return true;
	}
	else
	{
		return false;
	}*/
	if (g_frameData.vecEventData.size() > 0)
	{
		vector.swap(g_frameData.vecEventData);
		return true;
	}
	else
	{
		return false;
	}
}

void CeleX4::setVecSizeAndOverlap(unsigned long vecSize, unsigned long overlap)
{
	m_pDataProcessThread->getDataProcessor()->setVecSizeAndOverlap(vecSize, overlap);
}

bool CeleX4::getFixedNumEventDataVec(/*long length,long overlapLen, */std::vector<EventData> &vector, uint64_t& frameNo)
{
	//m_pDataProcessThread->getDataProcessor()->setVecSizeAndOverlap(length, overlapLen);
	
	if (g_fixedVecData.vecEventData.size() >0)
	{
		vector.swap(g_fixedVecData.vecEventData);
		frameNo = g_fixedVecData.frameNo;
		return true;
	}
	else
		return false;
}

bool CeleX4::getEventDataVector(std::vector<EventData> &vector, uint64_t& frameNo)
{
	if (g_frameData.vecEventData.size() > 0)
	{
		vector.swap(g_frameData.vecEventData);
		frameNo = g_frameData.frameNo;
		return true;
	}
	else
	{
		return false;
	}
}

void CeleX4::setEventFrameParameters(uint32_t frameTime, uint32_t intervalTime)
{
	m_pDataProcessThread->getDataProcessor()->setTimeSlice(intervalTime);
	if (m_bOpticalFlowEnable == true)
	{
		m_pDataProcessThread->getDataProcessor()->setMultiSliceTime(frameTime);
	}
	else
	{
		m_uiOverlapTime = frameTime - intervalTime;
		m_pDataProcessThread->getDataProcessor()->setOverlapTime(m_uiOverlapTime);
	}
}

cv::Mat CeleX4::getFullPicMat()
{
	CeleX4ProcessedData* pSensorData = getSensorDataObject();
	if (!pSensorData)
	{
		return cv::Mat();
	}
	cv::Mat mat = cv::Mat(cv::Size(768, 640), CV_8UC1, pSensorData->getFullPicBuffer());
	return mat;
}

cv::Mat CeleX4::getEventPicMat(emEventPicMode mode)
{
	CeleX4ProcessedData* pSensorData = getSensorDataObject();
	if (!pSensorData)
	{
		return cv::Mat();
	}
	cv::Mat mat = cv::Mat(cv::Size(768, 640), CV_8UC1, pSensorData->getEventPicBuffer(mode));
	return mat;
}

cv::Mat CeleX4::getOpticalFlowPicMat()
{
	if (isOpticalFlowEnabled())
	{
		CeleX4ProcessedData* pSensorData = getSensorDataObject();
		if (!pSensorData)
		{
			return cv::Mat();
		}
		cv::Mat mat = cv::Mat(cv::Size(768, 640), CV_8UC1, pSensorData->getOpticalFlowPicBuffer());
		return mat;
	}
	return cv::Mat();
}

cv::Mat CeleX4::getOpticalFlowDirectionPicMat()
{
	if (isOpticalFlowEnabled())
	{
		CeleX4ProcessedData* pSensorData = getSensorDataObject();
		if (!pSensorData)
		{
			return cv::Mat();
		}
		cv::Mat mat = cv::Mat(cv::Size(768, 640), CV_8UC1, pSensorData->getOpticalFlowDirectionPicBuffer());
		return mat;
	}
	return cv::Mat();
}

cv::Mat CeleX4::getOpticalFlowSpeedPicMat()
{
	if (isOpticalFlowEnabled())
	{
		CeleX4ProcessedData* pSensorData = getSensorDataObject();
		if (!pSensorData)
		{
			return cv::Mat();
		}
		cv::Mat mat = cv::Mat(cv::Size(768, 640), CV_8UC1, pSensorData->getOpticalFlowSpeedPicBuffer());
		return mat;
	}
	return cv::Mat();
}

bool CeleX4::openBinFile(std::string filePath)
{
	if (m_ifstreamBin.is_open())
		m_ifstreamBin.close();

	m_ifstreamBin.open(filePath.c_str(), std::ios::binary);
	if (!m_ifstreamBin.good())
	{
		std::cout << "Can't Open File: " << filePath.c_str();
		return false;
	}
	return true;
}

void CeleX4::convertBinToAVI(std::string binFile, emEventPicMode picMode, uint32_t frameTime, uint32_t intervalTime, cv::VideoWriter writer)
{
	isReadBin = true;
	int lenToRead = 4;
	unsigned char* data = (unsigned char*)malloc(lenToRead);
	cv::Mat img(640, 768, CV_8UC1);
	emSensorMode sensorMode = m_pDataProcessThread->getDataProcessor()->getBinFileAttributes(binFile).mode;
	m_pDataProcessThread->getDataProcessor()->setSensorMode(sensorMode);
	m_pDataProcessThread->getDataProcessor()->setTimeSlice(intervalTime);	//set interval time for event
	if (sensorMode == EventMode)
	{
		//if (picMode == EventOpticalFlowPic)
		//	m_pDataProcessThread->getDataProcessor()->setMultiSliceTime(frameTime);	//set frame time for optical flow
		//else
			m_pDataProcessThread->getDataProcessor()->setOverlapTime(frameTime - intervalTime);	//set overlaptime for event
	}

	openBinFile(binFile);

	while (!m_ifstreamBin.eof())
	{
		m_ifstreamBin.read(reinterpret_cast<char*>(data), sizeof(char) * 4);
		//process each event data
		m_pDataProcessThread->getDataProcessor()->processData(data, lenToRead);
		if (isCreateImage)
		{
			memcpy(img.data, m_pDataProcessThread->getDataProcessor()->getSensorDataObject()->getEventPicBuffer(picMode), sizeof(unsigned char) * 768 * 640);
			m_pDataProcessThread->getDataProcessor()->cleanEventBuffer();
			isCreateImage = false;
			writer.write(img.clone());
		}
	}
	writer.release();
}

void CeleX4::convertBinToAVI(std::string binFile, cv::VideoWriter writer)
{
	isReadBin = true;
	int lenToRead = 4;
	unsigned char* data = (unsigned char*)malloc(lenToRead);
	cv::Mat img(640, 768, CV_8UC1);
	emSensorMode sensorMode = m_pDataProcessThread->getDataProcessor()->getBinFileAttributes(binFile).mode;

	m_pDataProcessThread->getDataProcessor()->setSensorMode(sensorMode);

	openBinFile(binFile);

	while (!m_ifstreamBin.eof())
	{
		m_ifstreamBin.read(reinterpret_cast<char*>(data), sizeof(char) * 4);

		//process each event data
		m_pDataProcessThread->getDataProcessor()->processData(data, lenToRead);
		if (isCreateImage)
		{
			if (sensorMode == EventMode)
			{
				memcpy(img.data, m_pDataProcessThread->getDataProcessor()->getSensorDataObject()->getEventPicBuffer(EventAccumulatedPic), sizeof(unsigned char) * 768 * 640);
			}
			else
			{
				memcpy(img.data, m_pDataProcessThread->getDataProcessor()->getSensorDataObject()->getFullPicBuffer(), sizeof(unsigned char) * 768 * 640);
			}
			m_pDataProcessThread->getDataProcessor()->cleanEventBuffer();
			isCreateImage = false;
			writer.write(img.clone());
		}
	}
	writer.release();
}

void CeleX4::enableAutoAdjustBrightness(bool enable)
{
	m_bAutoAdjustBrightness = enable;
}

int CeleX4::getIMUDataSize()
{
	return m_pDataProcessThread->getDataProcessor()->getIMUDataSize();
}

int CeleX4::getIMUData(int count, std::vector<IMUData>& data)
{
	return m_pDataProcessThread->getDataProcessor()->getIMUData(count, data);
}

void CeleX4::setIMUIntervalTime(uint32_t value)
{
	//excuteCommand("SetIMU Interval Time");
	if (value > 255)
		value = 255;
	value = value << 24;
	FrontPanel::getInstance()->wireIn(0x02, value, 0xFF000000);
	//FrontPanel::getInstance()->wait(1);
	cout << "Address: " << 0x02 << "; Value: " << value << "; Mask: " << 0xFF000000 << endl;
}

int CeleX4::getIMUData(std::vector<IMUData>& data)
{
	return m_pDataProcessThread->getDataProcessor()->getIMUData(data);
}

bool CeleX4::denoisingByTimeInterval(std::vector<EventData> vec, cv::Mat &mat)
{
	cv::Mat preImg = cv::Mat::zeros(640, 768, CV_32FC1);

	if (!vec.empty())
	{
		mat = cv::Mat::zeros(cv::Size(768, 640), CV_8UC1);

		for (int t = 0; t < vec.size() - 1; ++t)
		{
			if (vec[t].row == 0 || vec[t].row == 639 || vec[t].col == 0 || vec[t].col == 767)
				continue;

			float d1 = preImg.at<float>(vec[t].row, vec[t].col);
			float d2 = preImg.at<float>(vec[t].row, vec[t].col + 1);
			float d3 = preImg.at<float>(vec[t].row + 1, vec[t].col);
			float d4 = preImg.at<float>(vec[t].row, vec[t].col - 1);
			float d5 = preImg.at<float>(vec[t].row - 1, vec[t].col);

			float deltaT = 5 * (vec[t].t + m_lPreT) - d1 - d2 - d3 - d4 - d5;

			m_lCount += 1;
			m_fTSum += deltaT;

			if (deltaT < m_fTSum / m_lCount / 2.0)
			{
				mat.at<uchar>(640 - vec[t].row - 1, 768 - vec[t].col - 1) = 255;
			}
			preImg.at<float>(vec[t].row, vec[t].col) = vec[t].t + m_lPreT;
			//cout << preImg.at<float>(vec[t].row, vec[t].col) << endl;
		}
		m_lPreT += vec[vec.size() - 1].t;
		return true;
	}
	if (m_fTSum > FLT_MAX || m_fTSum < FLT_MIN)
	{
		m_lCount = 0;
		m_fTSum = 0.0;
		m_lPreT = 0;
		preImg = cv::Mat::zeros(640, 768, CV_32FC1);
	}
}

bool CeleX4::denoisingAndCompresing(std::vector<EventData> vec, float compressRatio, cv::Mat &mat)
{
	cv::Mat preImg = cv::Mat::zeros(640, 768, CV_32FC1);
	m_MatCompressTemplateImg = cv::Mat::zeros(640, 768, CV_32FC1);

	if (!vec.empty())
	{
		mat = cv::Mat::zeros(cv::Size(768, 640), CV_8UC1);

		for (int t = 0; t < vec.size() - 1; ++t)
		{
			if (vec[t].row == 0 || vec[t].row == 639 || vec[t].col == 0 || vec[t].col == 767)
				continue;

			float d1 = preImg.at<float>(vec[t].row, vec[t].col);
			float d2 = preImg.at<float>(vec[t].row, vec[t].col + 1);
			float d3 = preImg.at<float>(vec[t].row + 1, vec[t].col);
			float d4 = preImg.at<float>(vec[t].row, vec[t].col - 1);
			float d5 = preImg.at<float>(vec[t].row - 1, vec[t].col);

			float deltaT = 5 * (vec[t].t + m_lPreT) - d1 - d2 - d3 - d4 - d5;

			m_lCount += 1;
			m_fTSum += deltaT;

			if (deltaT < m_fTSum / m_lCount / 2.0)
			{
				m_fDenoiseCount += 1;
				if (m_fCompressCount / m_fDenoiseCount > compressRatio)
				{
					m_fTao = m_fTao + m_iDelta;
				}
				else
				{
					m_fTao = m_fTao - m_iDelta;
					if (m_fTao < 0)
					{
						m_fTao = 0;
					}
				}
				//cout << tao << endl;		
				//cout << "tao: "<<tao << endl;
				if (vec[t].t + m_lPreT >= m_MatCompressTemplateImg.at<float>(vec[t].row, vec[t].col) + m_fTao)
				{
					m_fCompressCount = m_fCompressCount + 1;
					mat.at<uchar>(640 - vec[t].row - 1, 768 - vec[t].col - 1) = 255;
					m_MatCompressTemplateImg.at<float>(vec[t].row, vec[t].col) = vec[t].t + m_lPreT;
				}
			}
			preImg.at<float>(vec[t].row, vec[t].col) = vec[t].t + m_lPreT;
			//cout << preImg.at<float>(vec[t].row, vec[t].col) << endl;
		}
		m_lPreT += vec[vec.size() - 1].t;
		return true;
	}
	if (m_fTSum > FLT_MAX || m_fTSum < FLT_MIN)
	{
		m_lCount = 0;
		m_fTSum = 0.0;
		m_lPreT = 0;
		preImg = cv::Mat::zeros(640, 768, CV_32FC1);
	}
}
