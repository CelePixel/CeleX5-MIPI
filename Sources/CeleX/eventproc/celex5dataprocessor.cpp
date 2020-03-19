/*
* Copyright (c) 2017-2020  CelePixel Technology Co. Ltd.  All rights reserved.
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

#include <iostream>
#include <sstream>
#include "celex5dataprocessor.h"
#include "../base/filedirectory.h"

//#define _ENABLE_LOG_FILE_

const uint32_t g_uiMPDataFormat1MaxLen = 65536; // 2^16
const uint32_t g_uiMPDataFormat2MaxLen = 4096; // 2^12

static uint8_t s_uiWriteFrameIndex = 0;
static uint8_t s_uiReadFrameIndex = 1;

using namespace std;

CeleX5DataProcessor::CeleX5DataProcessor()
	: m_uiPixelCount(0)
	, m_uiPixelCountForEPS(0)
	, m_uiEventNumberEPS(0)
	, m_uiEventTCounter(0)
	, m_uiEventTCounterTotal(0)
	, m_uiEventTCounterEPS(0)
	//, m_uiEventTCountForShow(1500) //means 30ms
	, m_uiEventTCountForShow(2930) //means 30ms
	, m_uiEventTCountForRemove(0)
	//, m_uiEventTCountForEPS(50000) //means 1000ms
	, m_uiEventTCountForEPS(97656) //means 1000ms
	, m_uiEventCountForShow(100000)
	, m_uiEventRowCycleCountForShow(6)
	, m_iLastRow(-1)
	, m_iCurrentRow(-1)
	, m_uiRowCount(0)
	, m_uiEventCountSliceNum(5)
	, m_bIsGeneratingFPN(false)
	, m_iFpnCalculationTimes(-1)
	, m_bLoopModeEnabled(false)
	, m_uiISOLevel(2)
	, m_iLastRowTimestamp(-1)
	, m_iRowTimestamp(-1)
	, m_iMIPIDataFormat(2)
	, m_emSensorFixedMode(CeleX5::Event_Off_Pixel_Timestamp_Mode)
	, m_emSensorLoopAMode(CeleX5::Full_Picture_Mode)
	, m_emSensorLoopBMode(CeleX5::Event_Off_Pixel_Timestamp_Mode)
	, m_emSensorLoopCMode(CeleX5::Optical_Flow_Mode)
	, m_uiEventFrameTime(30000)
	, m_uiEventRowCycleCount(0)
	, m_emEventShowType(EventShowByTime)
	//, m_uiEventTUnitList{ 20, 25, 22, 25, 20, 17, 14, 25, 22, 20, 20, 20, 20, 14, 20, 20 }
	, m_uiEventTUnitN(1024) //1024=(15+1)*(15+1)*4
	, m_uiEventTUnitDList{ 80, 80, 90, 80, 100, 120, 140, 80, 90, 100, 110, 120, 130, 140, 150, 160 }
	//, m_uiCurrentEventTUnit(20)
	, m_uiCurrentEventTUnitD(100)
	, m_uiEventFrameNo(0)
	, m_uiEOTrampNo(1)
	, m_uiEventCountStep(30)
	, m_lCurrentEventFrameTimestamp(0)
	, m_lLastPackageTimestamp(0)
	, m_lFullFrameTimestamp(0)
	, m_lEventFrameTimestamp(0)
	, m_lOpticalFrameTimestamp(0)
	, m_uiPackageTCounter(0)
	, m_bSaveFullPicRawData(false)
	, m_iRotateType(0)
	, m_bFrameModuleEnabled(true)
	, m_bEventStreamEnabled(true)
	, m_bIMUModuleEnabled(true)
	, m_bEventDenoisingEnabled(false)
	, m_bFrameDenoisingEnabled(false)
	, m_bEventCountDensityEnabled(true)
	, m_bEventOpticalFlowEnabled(false)
	, m_bStartGenerateOFFPN(false)
	, m_iLastLoopNum(3)
	, m_emLastLoopMode(CeleX5::Unknown_Mode)
	, m_iFPNIndexForAdjustPic(1)
	, m_uiMinInPixelTimestamp(0)
	, m_uiMaxInPixelTimestamp(0)
{
	m_pCX5ProcessedData = new CeleX5ProcessedData;
	m_pCX5Server = new CX5SensorDataServer;
	m_pCX5Server->setCX5SensorData(m_pCX5ProcessedData);

	m_pEventCountBuffer = new uint8_t[CELEX5_PIXELS_NUMBER];
	m_pEventADCBuffer = new uint32_t[CELEX5_PIXELS_NUMBER];
	m_pLastADC = new uint32_t[CELEX5_PIXELS_NUMBER];
	m_pFpnGenerationBuffer = new long[CELEX5_PIXELS_NUMBER];
	m_pFpnBuffer = new int[CELEX5_PIXELS_NUMBER];
	m_pFpnBufferOF = new int[CELEX5_PIXELS_NUMBER];
	for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
	{
		m_pEventADCBuffer[i] = 0;
		m_pEventCountBuffer[i] = 0;
		m_pLastADC[i] = 0;
		m_pFpnGenerationBuffer[i] = 0;
		m_pFpnBuffer[i] = 0;
		m_pFpnBufferOF[i] = 0;
	}
	for (int i = 0; i < 10; i++)
	{
		m_pEventCountSlice[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventCountSlice[i], 0, CELEX5_PIXELS_NUMBER);
	}
	for (int i = 0; i < MAX_BUFFER_NUM; ++i)
	{
		m_pFullPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pFullPic[i], 0, CELEX5_PIXELS_NUMBER);

		m_pEventBinaryPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventBinaryPic[i], 0, CELEX5_PIXELS_NUMBER);
		m_pEventDenoisedBinaryPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventDenoisedBinaryPic[i], 0, CELEX5_PIXELS_NUMBER);

		m_pEventCountPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventCountPic[i], 0, CELEX5_PIXELS_NUMBER);
		m_pEventDenoisedCountPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventDenoisedCountPic[i], 0, CELEX5_PIXELS_NUMBER);

		m_pEventGrayPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventGrayPic[i], 0, CELEX5_PIXELS_NUMBER);

		m_pEventSuperimposedPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventSuperimposedPic[i], 0, CELEX5_PIXELS_NUMBER);

		m_pEventCountSlicePic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pEventCountSlicePic[i], 0, CELEX5_PIXELS_NUMBER);
		

		m_pOpticalFlowPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pOpticalFlowPic[i], 0, CELEX5_PIXELS_NUMBER);
		m_pOpticalFlowSpeedPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pOpticalFlowSpeedPic[i], 0, CELEX5_PIXELS_NUMBER);
		m_pOpticalFlowDirectionPic[i] = new uint8_t[CELEX5_PIXELS_NUMBER];
		memset(m_pOpticalFlowDirectionPic[i], 0, CELEX5_PIXELS_NUMBER);
	}

	m_pEventAccumulatedPic = new uint8_t[CELEX5_PIXELS_NUMBER];
	memset(m_pEventAccumulatedPic, 0, CELEX5_PIXELS_NUMBER);

	loadOpticalFlowFPN();

#ifdef _ENABLE_LOG_FILE_
	if (!m_ofLogFile.is_open())
		m_ofLogFile.open("log_file_time.txt");
#endif

	m_pOFFPNEventLatestValue = new uint16_t[CELEX5_PIXELS_NUMBER];
	memset(m_pOFFPNEventLatestValue, 0, CELEX5_PIXELS_NUMBER*2);
}

CeleX5DataProcessor::~CeleX5DataProcessor()
{
	if (m_pCX5ProcessedData) delete m_pCX5ProcessedData;
	if (m_pCX5Server) delete m_pCX5Server;

	if (m_pEventCountBuffer) delete[] m_pEventCountBuffer;
	if (m_pEventADCBuffer) delete[] m_pEventADCBuffer;
	if (m_pLastADC) delete[] m_pLastADC;
	if (m_pFpnGenerationBuffer) delete[] m_pFpnGenerationBuffer;
	if (m_pFpnBuffer) delete[] m_pFpnBuffer;
	if (m_pFpnBufferOF) delete[] m_pFpnBufferOF;

	for (int i = 0; i < 10; i++)
	{
		if (m_pEventCountSlice) delete[] m_pEventCountSlice[i];
	}

	for (int i = 0; i < MAX_BUFFER_NUM; ++i)
	{
		if (m_pFullPic[i]) delete[] m_pFullPic[i];

		if (m_pEventBinaryPic[i]) delete[] m_pEventBinaryPic[i];
		if (m_pEventDenoisedBinaryPic[i]) delete[] m_pEventDenoisedBinaryPic[i];
		if (m_pEventCountPic[i]) delete[] m_pEventCountPic[i];
		if (m_pEventDenoisedCountPic[i]) delete[] m_pEventDenoisedCountPic[i];
		if (m_pEventGrayPic[i]) delete[] m_pEventGrayPic[i];
		//if (m_pEventAccumulatedPic[i]) delete[] m_pEventAccumulatedPic[i];
		if (m_pEventSuperimposedPic[i]) delete[] m_pEventSuperimposedPic[i];
		if (m_pEventCountSlicePic[i]) delete[] m_pEventCountSlicePic[i];
		
		if (m_pOpticalFlowPic[i]) delete[] m_pOpticalFlowPic[i];
		if (m_pOpticalFlowSpeedPic[i]) delete[] m_pOpticalFlowSpeedPic[i];
		if (m_pOpticalFlowDirectionPic[i]) delete[] m_pOpticalFlowDirectionPic[i];
	}
	if (m_pEventAccumulatedPic) delete[] m_pEventAccumulatedPic;
}

/*
*  @function: getFullPicBuffer
*  @brief   : obtain a full-frame picture buffer
*  @input   :  
*  @output  : buffer: frame buffer pointer (size is 1280 * 800)
*  @return  : 
*/
void CeleX5DataProcessor::getFullPicBuffer(uint8_t* buffer)
{
	memcpy(buffer, m_pFullPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
}

/*
*  @function: getFullPicBuffer
*  @brief   : obtain a full-frame picture buffer and pc timestamp
*  @input   :
*  @output  : buffer: frame buffer pointer (size is 1280 * 800)
*             timestamp: the pc timestamp when this frame buffer was received from sensor
*  @return  :
*/
void CeleX5DataProcessor::getFullPicBuffer(uint8_t* buffer, std::time_t& timestamp)
{
	memcpy(buffer, m_pFullPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	timestamp = m_lFullFrameTimestamp;
}

/*
*  @function: getEventPicBuffer
*  @brief   : obtain an event frame buffer and pc timestamp
*  @input   : type: event frame buffer type
*  @output  : buffer: frame buffer pointer (size is 1280 * 800)
*  @return  :
*/
void CeleX5DataProcessor::getEventPicBuffer(uint8_t* buffer, CeleX5::EventPicType type)
{
	if (type == CeleX5::EventBinaryPic)
		memcpy(buffer, m_pEventBinaryPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventGrayPic)
		memcpy(buffer, m_pEventGrayPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventAccumulatedPic)
		memcpy(buffer, m_pEventAccumulatedPic, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventSuperimposedPic)
		memcpy(buffer, m_pEventSuperimposedPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedBinaryPic)
		memcpy(buffer, m_pEventDenoisedBinaryPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountPic)
		memcpy(buffer, m_pEventCountPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedCountPic)
		memcpy(buffer, m_pEventDenoisedCountPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountDensityPic)
		memcpy(buffer, m_pEventCountSlicePic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventInPixelTimestampPic)
		memcpy(buffer, m_pOpticalFlowPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
}

/*
*  @function: getEventPicBuffer
*  @brief   : obtain an event frame buffer and 
*  @input   : type: event frame buffer type
*  @output  : buffer: frame buffer pointer (size is 1280 * 800)
*             timestamp: the pc timestamp when this frame buffer was received from sensor
*  @return  :
*/
void CeleX5DataProcessor::getEventPicBuffer(uint8_t* buffer, std::time_t& timestamp, CeleX5::EventPicType type)
{
	if (type == CeleX5::EventBinaryPic)
		memcpy(buffer, m_pEventBinaryPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventGrayPic)
		memcpy(buffer, m_pEventGrayPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventAccumulatedPic)
		memcpy(buffer, m_pEventAccumulatedPic, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventSuperimposedPic)
		memcpy(buffer, m_pEventSuperimposedPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedBinaryPic)
		memcpy(buffer, m_pEventDenoisedBinaryPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountPic)
		memcpy(buffer, m_pEventCountPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedCountPic)
		memcpy(buffer, m_pEventDenoisedCountPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountDensityPic)
		memcpy(buffer, m_pEventCountSlicePic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventInPixelTimestampPic)
		memcpy(buffer, m_pOpticalFlowPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);

	if (m_bLoopModeEnabled)
		timestamp = m_lEventFrameTimestamp;
	else
		timestamp = m_lCurrentEventFrameTimestamp;
}

/*
*  @function: getOpticalFlowPicBuffer
*  @brief   : obtain an optical-flow related frame buffer
*  @input   : type: event frame buffer type
*  @output  : buffer: frame buffer pointer (size is 1280 * 800)
*  @return  :
*/
void CeleX5DataProcessor::getOpticalFlowPicBuffer(uint8_t* buffer, CeleX5::OpticalFlowPicType type)
{
	if (type == CeleX5::OpticalFlowPic)
		memcpy(buffer, m_pOpticalFlowPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::OpticalFlowSpeedPic)
		memcpy(buffer, m_pOpticalFlowSpeedPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::OpticalFlowDirectionPic)
		memcpy(buffer, m_pOpticalFlowDirectionPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
}

/*
*  @function: getOpticalFlowPicBuffer
*  @brief   : obtain an optical-flow related frame buffer and pc timestamp
*  @input   : type: optical-flow frame buffer type
*  @output  : buffer: frame buffer pointer (size is 1280 * 800)
*             timestamp: the pc timestamp when this frame buffer was received from sensor
*  @return  :
*/
void CeleX5DataProcessor::getOpticalFlowPicBuffer(uint8_t* buffer, std::time_t& timestamp, CeleX5::OpticalFlowPicType type)
{
	if (type == CeleX5::OpticalFlowPic)
		memcpy(buffer, m_pOpticalFlowPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::OpticalFlowSpeedPic)
		memcpy(buffer, m_pOpticalFlowSpeedPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::OpticalFlowDirectionPic)
		memcpy(buffer, m_pOpticalFlowDirectionPic[s_uiReadFrameIndex], CELEX5_PIXELS_NUMBER);

	timestamp = m_lOpticalFrameTimestamp;
}

/*
*  @function: getEventDataVector
*  @brief   : obtain an event data stream over a period time
*  @input   : 
*  @output  : vecEvent: event data vector over a period time
*  @return  :
*/
bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vecEvent)
{
	vecEvent = m_vecEventData;
	return true;
}

/*
*  @function: getEventDataVector
*  @brief   : obtain an event data stream over a period time and the frame number
*  @input   : 
*  @output  : vecEvent: frame buffer (size is 1280 * 800)
*             frameNo : frame number
*  @return  :
*/
bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vecEvent, uint32_t& frameNo)
{
	vecEvent = m_vecEventData;
	frameNo = m_uiEventFrameNo;
	return true;
}

/*
*  @function: getEventDataVector
*  @brief   : obtain an event data stream over a period time and the frame number
*  @input   :
*  @output  : vecEvent : frame buffer (size is 1280 * 800)
*             frameNo  : the frame number of the event vector 
*             timestamp: the pc timestamp when the event data stream was received from sensor
*  @return  :
*/
bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vecEvent, uint32_t& frameNo, std::time_t& timestamp)
{
	vecEvent = m_vecEventData;
	frameNo = m_uiEventFrameNo;
	if (m_bLoopModeEnabled)
		timestamp = m_lEventFrameTimestamp;
	else
		timestamp = m_lCurrentEventFrameTimestamp;
	return true;
}

/*
*  @function: processMIPIData
*  @brief   : parse the raw data of sensor
*  @input   : pData: a package of sensor raw data
*             dataSize: package size
*             time_stamp_end: the pc timestamp when this pacakge data was received from sensor
*             imu_data: imu data vecotr
*  @output  :
*  @return  :
*/
//vecData[size -1]: data mode = 1: event / 0: fullpic
void CeleX5DataProcessor::processMIPIData(uint8_t* pData, uint32_t dataSize, std::time_t time_stamp_end, std::vector<IMURawData>& imuData)
{
	//cout << "CeleX5DataProcessor::processData: time_stamp_end = " << time_stamp_end << endl;
	int size = imuData.size();
	//cout << "CeleX5DataProcessor::processData: imu_data size = " << size << endl;
	for (int i = 0; i < size; i++)
	{
		m_vectorIMURawData.push_back(imuData[i]);
		//cout << "--- CeleX5DataProcessor::processData: imu_time_stamp = " << imu_data[i].time_stamp << endl;
	}
	//cout << "CeleX5DataProcessor::processData: data mode ================= " << (int)*(pData + dataSize -1) << endl;
	if (*(pData + dataSize - 1) == 0 /*|| dataSize > 500000*/) //full pic mode: package size = 1536001 = 1280 * 800 * 1.5 + 1
	{
		uint8_t mode = 0xFF & *pData;
		if (mode == 0x60 || mode == 0x80 || mode == 0xA0 || mode == 0xC0 || mode == 0xE0)
		{
			m_emCurrentSensorMode = CeleX5::CeleX5Mode((0xE0 & *pData) >> 5);
			//cout << "CeleX5DataProcessor::processData: m_emCurrentSensorMode = " << (int)m_emCurrentSensorMode << endl;
			processFullPicData(pData, dataSize, time_stamp_end);
		}
	}
	else
	{
		if (m_iMIPIDataFormat == 0)// Format0 (CSR_73 = 0), Package Size: 1024*200*1.5 = 307200 Byte
		{
			parseEventDataFormat0(pData, dataSize, time_stamp_end);
		}
		else if (m_iMIPIDataFormat == 1)// Format1 (CSR_73 = 1), Package Size: 1024*200*1.75 = 358400 Byte
		{
			parseEventDataFormat1(pData, dataSize, time_stamp_end);
		}
		else if (m_iMIPIDataFormat == 2)// Format2 (CSR_73 = 2), Package Size: 1024*200*1.75 = 358400 Byte
		{
			parseEventDataFormat2(pData, dataSize, time_stamp_end);
		}
	}
	m_lLastPackageTimestamp = time_stamp_end;
}

/*
*  @function: disableFrameModule
*  @brief   : disable the Create Image Frame module
*             If you just want to obtain (x,y,A,t) stream, you cound disable this function to imporve performance.
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::disableFrameModule()
{
	m_bFrameModuleEnabled = false;
}

/*
*  @function: enableFrameModule
*  @brief   : enable the Create Image Frame module
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::enableFrameModule()
{
	m_bFrameModuleEnabled = true;
}

/*
*  @function: isFrameModuleEnabled
*  @brief   : check if the Create Image Frame module is enabled
*  @input   :
*  @output  :
*  @return  :
*/
bool CeleX5DataProcessor::isFrameModuleEnabled()
{
	return m_bFrameModuleEnabled;
}

/*
*  @function: disableEventStreamModule
*  @brief   : disable the Event Stream module
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::disableEventStreamModule()
{
	m_bEventStreamEnabled = false;
}

/*
*  @function: enableEventStreamModule
*  @brief   : enable the Event Stream module
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::enableEventStreamModule()
{
	m_bEventStreamEnabled = true;
}

/*
*  @function: isEventStreamEnabled
*  @brief   : check if the Event Stream module is enabled
*  @input   :
*  @output  :
*  @return  :
*/
bool CeleX5DataProcessor::isEventStreamEnabled()
{
	return m_bEventStreamEnabled;
}

/*
*  @function: disableFrameModule
*  @brief   : disable the IMU module
*             If you don't want to obtain IMU data, you cound disable this function to imporve performance.
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::disableIMUModule()
{
	m_bIMUModuleEnabled = false;
}

/*
*  @function: enableFrameModule
*  @brief   : enable the IMU module
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::enableIMUModule()
{
	m_bIMUModuleEnabled = true;
}

/*
*  @function: isIMUModuleEnabled
*  @brief   : check if the IMU module is enabled
*  @input   :
*  @output  :
*  @return  :
*/
bool CeleX5DataProcessor::isIMUModuleEnabled()
{
	return m_bIMUModuleEnabled;
}

void CeleX5DataProcessor::disableEventDenoising()
{
	m_bEventDenoisingEnabled = false;
}

void CeleX5DataProcessor::enableEventDenoising()
{
	m_bEventDenoisingEnabled = true;
}

bool CeleX5DataProcessor::isEventDenoisingEnabled()
{
	return m_bEventDenoisingEnabled;
}

void CeleX5DataProcessor::disableFrameDenoising()
{
	m_bFrameDenoisingEnabled = false;
}

void CeleX5DataProcessor::enableFrameDenoising()
{
	m_bFrameDenoisingEnabled = true;
}

bool CeleX5DataProcessor::isFrameDenoisingEnabled()
{
	return m_bFrameDenoisingEnabled;
}

void CeleX5DataProcessor::disableEventCountSlice()
{
	m_bEventCountDensityEnabled = false;
}

void CeleX5DataProcessor::enableEventCountSlice()
{
	m_bEventCountDensityEnabled = true;
}

bool CeleX5DataProcessor::isEventCountSliceEnabled()
{
	return m_bEventCountDensityEnabled;
}

void CeleX5DataProcessor::disableEventOpticalFlow()
{
	m_bEventOpticalFlowEnabled = false;
}

void CeleX5DataProcessor::enableEventOpticalFlow()
{
	m_bEventOpticalFlowEnabled = true;
}

bool CeleX5DataProcessor::isEventOpticalFlowEnabled()
{
	return m_bEventOpticalFlowEnabled;
}

/*
*  @function: processFullPicData
*  @brief   : parse the raw full-frame data
*  @input   : pData: a package of full-frame data
*             dataSize: package size (default size is 1536001, if the size is not this value, there may be some thing wrong with this package data)
*             time_stamp_end: the pc timestamp when this pacakge data was received from sensor
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::processFullPicData(uint8_t* pData, int dataSize, std::time_t timestampEnd)
{
	m_uiPixelCount = 0;
	m_uiRowCount = 0;
	m_uiEventRowCycleCount = 0;
	m_uiEventTCounter = 0;
	if (m_bLoopModeEnabled)
	{
		m_uiEventTCounterTotal = 0;
	}
	m_iLastRowTimestamp = -1;
	m_iRowTimestamp = -1;

	if (dataSize != 1536001)
	{
		//cout << "CeleX5DataProcessor::processFullPicData: Not a full package: " << dataSize << endl;
		return;
	}

	//------ for test ------
	if (m_bSaveFullPicRawData)
	{
		saveFullPicRawData(pData);
	} //------ for test ------

#ifdef _ENABLE_LOG_FILE_
#ifdef _OPENMP
	omp_set_num_threads(2);
	cout << "max thread: " << omp_get_max_threads() << endl;
	double begin_op_t = omp_get_wtime();
	double begin_t = clock();
#else
	clock_t begin_t, poll_t;
	begin_t = clock();
#endif
#endif //_ENABLE_LOG_FILE_

#ifdef _OPENMP
#pragma omp parallel num_threads(2)
#endif
	int index = 0;
	for (int i = 0; i < dataSize - 2; i += 3)
	{
		uint8_t value1 = *(pData + i);
		uint8_t value2 = *(pData + i + 1);
		uint8_t value3 = *(pData + i + 2);

		if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode ||
			m_emCurrentSensorMode == CeleX5::Optical_Flow_Mode ||
			m_emCurrentSensorMode == CeleX5::Multi_Read_Optical_Flow_Mode)
		{
			//--- 8-bits ---
			uint8_t adc1 = value1;
			uint8_t adc2 = value2;

			m_pEventADCBuffer[index] = 255 - adc1;
			m_pEventADCBuffer[index + 1] = 255 - adc2;
		}
		else if (m_emCurrentSensorMode == CeleX5::Optical_Flow_FPN_Mode)
		{
			//--- 12-bits ---
			uint16_t adc11 = (value1 << 4) + (0x0F & value3);
			uint16_t adc22 = (value2 << 4) + ((0xF0 & value3) >> 4);

			m_pEventADCBuffer[index] = 4095 - adc11;
			m_pEventADCBuffer[index + 1] = 4095 - adc22;
		}
		index += 2;
		if (index == CELEX5_PIXELS_NUMBER)
		{
			break;
		}
	}
	if (m_bIsGeneratingFPN)
	{
		generateFPNimpl();
	}
	else
	{
		if (m_bIMUModuleEnabled)
		{
			parseIMUData(timestampEnd);
		}
		if (m_bFrameModuleEnabled)
		{
			createImage(timestampEnd);
		}
		else
		{
			m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
		}
		m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
	}
#ifdef _ENABLE_LOG_FILE_
#ifdef _OPENMP
	double poll_op_t = omp_get_wtime();
	m_ofLogFile << "  Elapsed Time: " << (poll_op_t - begin_op_t) * CLOCKS_PER_SEC << " (ms)" << std::endl;
#else
	poll_t = clock();
	m_ofLogFile << "  Elapsed Time: " << (poll_t - begin_t) << " (ms)" << std::endl;
#endif
#endif //_ENABLE_LOG_FILE_
}

/*
*  @function: parseEventDataFormat0
*  @brief   : parse the event format0 data, this format is not used because there is an error with the adc value output from the sensor
*  @input   : pData: a package of event format0 data
*             dataSize: package size (default size is 357001, if the size is not this value, there may be some thing wrong with this package data)
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::parseEventDataFormat0(uint8_t* pData, int dataSize, std::time_t timestampEnd)
{
	//cout << __FUNCTION__ << ": dataSize = " << dataSize << endl;
	if (dataSize != 307201)
	{
		//cout << "CeleX5DataProcessor::parseEventDataFormat0: Not a full package!" << endl;
	}
	int index = 0;
	int row = 0;
	int col = 0;
	int adc = 0;
	int temperature = 0;
	uint32_t row_time_stamp = 0;
	for (int i = 0; i < dataSize - 2; i += 3)
	{
		uint8_t value1 = pData[i];
		uint8_t value2 = pData[i + 1];
		uint8_t value3 = pData[i + 2];

		//dataID[1:0] = data[1:0] = value3[1:0]
		uint8_t dataID = (0x03 & value3);

		if (dataID == 2) //b2'10: row_addr[9:0] & row_time_stamp[11:0]
		{
			//row[9:0] = data[23:14] = value2[7:0] + value3[7:6]
			row = (value2 << 2) + ((0xC0 & value3) >> 6);

			//row_time_stamp[11:0] = data[13:2] = value3[5:4] + value1[7:0] + value3[3:2]
			row_time_stamp = ((0x30 & value3) << 6) + (value1 << 2) + ((0x0C & value3) >> 2);
			//cout << "Format 0: " << "row = " << row << ", row_time_stamp = " << row_time_stamp << endl;
		}
		else if (dataID == 1) //b2'01: col_addr[10:0] & adc[7:0]
		{
			//adc[7:0] = data[23:16] = value2[7:0]
			adc = value2;

			//col[10:0] = data[15:5] = value3[7:4] + value1[7:1]
			col = ((0xF0 & value3) << 3) + ((0xFE & value1) >> 1);

			//packet_format[2:0] = data[4:2] = value1[0] + value3[3:2]
			int packet_format = ((0x01 & value1) << 2) + ((0x0C & value3) >> 2);
			//cout << "format 0: " << "adc = " << adc << ", col = " << col << ", packet_format = " << packet_format << endl;

			index = row * CELEX5_COL + col;
			/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
			m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
			m_pEventCountBuffer[index] += 1;
			m_pEventADCBuffer[index] = adc;
		}
		else if (dataID == 0)
		{
			//mark[6:0] = data[23:17] = value2[7:1] = 2b'0000000
			int mark = (0xFE & value2) >> 1;

			//op_mode[2:0] = data[16:14] = value2[0] + value3[7:6]
			int op_mode = ((0x01 & value2) << 2) + ((0xC0 & value3) >> 6);
			m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;

			//temperature[11:0] = data[13:2] = value3[5:4] + value1[7:0] + value3[3:2]
			temperature = ((0x30 & value3) << 6) + (value1 << 2) + ((0x0C & value3) >> 2);

			//cout << "Format 0:  mark = " << mark << ", op_mode = " << op_mode << ", temperature = " << temperature << endl;
		}
	}
	if (m_bFrameModuleEnabled)
	{
		createImage(0);
	}
	else
	{
		m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
	}
	m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
}

/*
*  @function: parseEventDataFormat1
*  @brief   : parse the event format1 data, this format is used in Event Intensity mode and Event In-Pixel Timestamp mode
*  @input   : pData: a package of event format0 data
*             dataSize: package size (default size is 357001, if the size is not this value, there may be some thing wrong with this package data)
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::parseEventDataFormat1(uint8_t* pData, int dataSize, std::time_t timestampEnd)
{
	if (m_bLoopModeEnabled)
	{
		if (findModeInLoopGroup(CeleX5::Event_In_Pixel_Timestamp_Mode))
			m_emCurrentSensorMode = CeleX5::Event_In_Pixel_Timestamp_Mode;
		else if (findModeInLoopGroup(CeleX5::Event_Intensity_Mode))
			m_emCurrentSensorMode = CeleX5::Event_Intensity_Mode;
	}
	//cout << "CeleX5DataProcessor::parseEventDataFormat1: dataSize = " << dataSize << endl;
	if (!m_bLoopModeEnabled)
	{
		if (dataSize != 357001) //358401
		{
			cout << "CeleX5DataProcessor::parseEventDataFormat1: Not a full package: " << dataSize << endl;
			return;
		}
	}
	int index = 0;
	int col = 0;
	int adc = 0;
	int temperature = 0;
	for (int i = 0; i < dataSize - 6; i += 7)
	{
		uint8_t value1 = *(pData + i);
		uint8_t value2 = *(pData + i + 1);
		uint8_t value3 = *(pData + i + 2);
		uint8_t value4 = *(pData + i + 3);
		uint8_t value5 = *(pData + i + 4);
		uint8_t value6 = *(pData + i + 5);
		uint8_t value7 = *(pData + i + 6);
		//---------- dataID1[1:0] = data[1:0] = value5[1:0] ----------
		uint8_t dataID1 = 0x03 & value5;
		if (dataID1 == 2) //2b'10
		{
			//-------- event row --------
			m_iLastRow = m_iCurrentRow;
			//row_addr[9:0] = data_28_1[27:18] = value2[7:0] + value6[3:2]
			m_iCurrentRow = (value2 << 2) + ((0x0C & value6) >> 2);
			m_uiRowCount++;
			if (m_iCurrentRow < m_iLastRow)
			{
				//cout << "currentRow = " << m_iCurrentRow << ", lastRow = " << m_iLastRow << endl;
				m_uiEventRowCycleCount++;
			}
			if (m_bEventDenoisingEnabled)
			{
				denoisedPerRow(true);
			}
			//-------- event time stamp --------
			m_iLastRowTimestamp = m_iRowTimestamp;
			//row_time_stamp[15:0] = data_28_1[17:2] = value6[1:0] + value5[7:6] + value1[7:0] + value5[5:2]
			m_iRowTimestamp = ((0x03 & value6) << 14) + ((0xC0 & value5) << 6) + (value1 << 4) + ((0x3C & value5) >> 2);
			//cout << "Format 1 (Package_A): " << "row = " << m_iCurrentRow << ", row_time_stamp = " << m_iRowTimeStamp << endl;
			processMIPIEventTimestamp();
		}
		else if (dataID1 == 1) //2b'01
		{
			//adc[11:0] = data_28_1[27:16] = value2[7:0] + value6[3:0]
			adc = 4095 - ((value2 << 4) + (0x0F & value6)); //for 8-bit: adc = value2

			//col[10:0] = data_28_1[15:5] = value5[7:6] + value1[7:0] + value5[5]
			col = ((0xC0 & value5) << 3) + (value1 << 1) + ((0x20 & value5) >> 5);

			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{	
				if (CeleX5::Event_Intensity_Mode == m_emCurrentSensorMode)
				{
					saveIntensityEvent(col, adc, 255 - value2);
				}
				else if (CeleX5::Event_In_Pixel_Timestamp_Mode == m_emCurrentSensorMode)
				{
					saveOpticalFlowEvent(col, adc, 255 - value2);
				}
			}
		}
		else if (dataID1 == 0) //2b'00
		{
			//mark[10:0] = data_28_1[27:17] = value2[7:0] + value6[3:1] = 2b'0000 0000 000
			int mark = (value2 << 3) + ((0x0E & value6) >> 1);

			//op_mode[2:0] = data_28_1[16:14] = value6[0] + value5[7:6]
			int op_mode = ((0x01 & value6) << 2) + ((0xC0 & value5) >> 6);
			if (!m_bLoopModeEnabled)
				m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;

			////temperature[11:0] = data_28_1[13:2] = value1[7:0] + value5[5:2]
			//temperature = (value1 << 4) + ((0x3C & value5) >> 2);
			//m_pCX5ProcessedData->setTemperature(temperature);
			////cout << "Format 1 (Package_A): mark = " << mark << ", op_mode = " << op_mode << ", temperature = " << temperature << endl;
		}

		//---------- dataID2[1:0] = data[1:0] = value6[5:4] ---------- 
		uint8_t dataID2 = (0x30 & value6) >> 4;
		if (dataID2 == 2) //2b'10
		{
			//-------- event row --------
			m_iLastRow = m_iCurrentRow;
			//row_addr[9:0] = data_28_2[27:18] = (value4[7:0] << 2) + (value7[7:6] >> 6)
			m_iCurrentRow = (value4 << 2) + ((0xC0 & value7) >> 6);
			m_uiRowCount++;
			if (m_iCurrentRow < m_iLastRow)
			{
				//cout << "currentRow = " << m_iCurrentRow << ", lastRow = " << m_iLastRow << endl;
				m_uiEventRowCycleCount++;
			}
			if (m_bEventDenoisingEnabled)
			{
				denoisedPerRow(true);
			}
			//-------- event time stamp --------
			m_iLastRowTimestamp = m_iRowTimestamp;
			//row_time_stamp[15:0] = data_28_1[17:2] = value7[5:2] + value3[7:0] + value7[1:0] + value6[7:6]
			m_iRowTimestamp = ((0x3C & value7) << 10) + (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//cout << "Format 1 (Package_B): " << "row = " << m_iCurrentRow << ", row_time_stamp = " << m_iRowTimeStamp << endl;
			processMIPIEventTimestamp();
		}
		else if (dataID2 == 1) //2b'01
		{
			//adc[11:0] = data_28_2[27:16] = value4[7:0] + value7[7:4]
			adc = 4095 - ((value4 << 4) + ((0xF0 & value7) >> 4)); //for 8-bit: adc = value4

			//col[10:0] = data_28_2[15:5] = value7[3:2] + value3[7:0]  + value7[1]
			col = ((0x0C & value7) << 7) + (value3 << 1) + ((0x02 & value7) >> 1);

			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				if (CeleX5::Event_Intensity_Mode == m_emCurrentSensorMode)
				{
					saveIntensityEvent(col, adc, 255 - value4);
				}
				else if (CeleX5::Event_In_Pixel_Timestamp_Mode == m_emCurrentSensorMode)
				{
					saveOpticalFlowEvent(col, adc, 255 - value4);
				}			
			}
		}
		else if (dataID2 == 0) //2b'00
		{
			//mark[10:0] = data_28_2[27:17] = value4[7:0] + value7[7:5] = 2b'0000 0000 000
			int mark = (value4 << 3) + ((0xE0 & value7) >> 5);

			//op_mode[2:0] = data_28_2[16:14] = value7[4:2]
			int op_mode = ((0x1C & value7) >> 2);
			if (!m_bLoopModeEnabled)
				m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;
			//Notes: when sensor works in loop mode, the op_mode is wrong.

			////temperature[11:0] = data[13:2] = value3[7:0] + value7[1:0] + value6[7:6]
			//temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//m_pCX5ProcessedData->setTemperature(temperature);
			////cout << "Format 1 (Package_B): mark = " << mark << ", op_mode = " << op_mode << ", temperature = " << temperature << endl;
		}
	}
	m_uiPackageTCounter = 0;

	if (m_bLoopModeEnabled)
	{
		if (dataSize < 357001) //the last package of event data
		{
			m_uiEventFrameNo++;

			if (m_bIMUModuleEnabled)
			{
				parseIMUData(m_lLastPackageTimestamp);
			}
			if (m_bFrameModuleEnabled)
			{
				createImage(timestampEnd);
			}
			else
			{
				m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
			}
			m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);

			m_uiPixelCount = 0;
			m_uiRowCount = 0;
			m_uiEventRowCycleCount = 0;
			m_uiEventTCounter = 0;
			m_vecEventData.clear();
		}
	}
}

/*
*  @function: parseEventDataFormat2
*  @brief   : parse the event format2 data, this format can only be used in Event Off-Pixel Timestamp mode because there is no adc in this format
*  @input   : pData: a package of event format0 data
*             dataSize: package size (default size is 357001, if the size is not this value, there may be some thing wrong with this package data)
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::parseEventDataFormat2(uint8_t* pData, int dataSize, std::time_t timestampEnd)
{
	//cout << __FUNCTION__ << ": dataSize = " << dataSize << endl;
	if (!m_bLoopModeEnabled)
	{
		if (dataSize != 357001) //358401
		{
			//cout << "CeleX5DataProcessor::parseEventDataFormat2: Not a full package: " << dataSize << endl;
			return;
		}
	}
#ifdef _ENABLE_LOG_FILE_
	clock_t begin_t, poll_t;
	begin_t = clock();
#endif // _ENABLE_LOG_FILE_

	int col = 0;
	int temperature = 0;
	m_emCurrentSensorMode = CeleX5::Event_Off_Pixel_Timestamp_Mode;
	for (int i = 0; i < dataSize - 6; i += 7)
	{
		uint8_t value1 = *(pData + i);
		uint8_t value2 = *(pData + i + 1);
		uint8_t value3 = *(pData + i + 2);
		uint8_t value4 = *(pData + i + 3);
		uint8_t value5 = *(pData + i + 4);
		uint8_t value6 = *(pData + i + 5);
		uint8_t value7 = *(pData + i + 6);

		//---------- dataID1[1:0] = data[1:0] = value5[1:0] ----------
		uint8_t dataID1 = 0x03 & value5;
		if (dataID1 == 0x02) //row data
		{
			m_iLastRow = m_iCurrentRow;

			//row_addr[9:0] = data_14_1[13:4] = value1[7:0] + value5[5:4]
			m_iCurrentRow = (value1 << 2) + ((0x30 & value5) >> 4);
			//cout << "Package A: " << "row = " << m_iCurrentRow << endl;

			m_uiRowCount++;
			if (m_iCurrentRow < m_iLastRow)
			{
				//cout << "currentRow = " << m_iCurrentRow << ", lastRow = " << m_iLastRow << endl;
				m_uiEventRowCycleCount++;
			}

			if (m_bEventDenoisingEnabled)
			{
				denoisedPerRow(false);
			}
		}
		else if (dataID1 == 0x01) //col data
		{
			//col_addr[10:0] = data_14_1[13:3] = value1[7:0] + value5[5:3]
			col = (value1 << 3) + ((0x38 & value5) >> 3);
			//cout << "--- Package A: " << "col = " << col << endl;
			saveFormat2Event(col, 0);
		}
		//else if (dataID1 == 0x00) // temperature data
		//{
		//	//temperature[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
		//	temperature = (value1 << 4) + ((0x3C & value5) >> 2);
		//	m_pCX5ProcessedData->setTemperature(temperature);
		//	//cout << "Format 2 time full (Package_A): temperature = " << temperature << endl;
		//}
		else if (dataID1 == 0x03) //timestamp data
		{
			m_iLastRowTimestamp = m_iRowTimestamp;

			//row_time_stamp[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
			m_iRowTimestamp = (value1 << 4) + ((0x3C & value5) >> 2);
			//cout << "Format 2 (Package_A): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimestamp();
		}

		//---------- dataID2[1:0] = data[1:0] = value5[7:6] ----------
		uint8_t dataID2 = 0xC0 & value5;
		if (dataID2 == 0x80) //row data
		{
			m_iLastRow = m_iCurrentRow;
			//row_addr[9:0] = data_14_2[13:4] = value2[7:0] + value6[3:2]
			m_iCurrentRow = (value2 << 2) + ((0x0C & value6) >> 2);
			//cout << "Package B: " << "row = " << m_iCurrentRow << endl;
			m_uiRowCount++;
			if (m_iCurrentRow < m_iLastRow)
			{
				//cout << "currentRow = " << m_iCurrentRow << ", lastRow = " << m_iLastRow << endl;
				m_uiEventRowCycleCount++;
			}
			if (m_bEventDenoisingEnabled)
			{
				denoisedPerRow(false);
			}
		}
		else if (dataID2 == 0x40) //col data
		{
			//col_addr[10:0] = data_14_2[13:3] = value2[7:0] + value6[3:1]
			col = (value2 << 3) + ((0x0E & value6) >> 1);
			//cout << "--- Package B: " << "col = " << col << endl;
			saveFormat2Event(col, 0);
		}
		//else if (dataID2 == 0x00) //temperature data
		//{
		//	//temperature[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
		//	temperature = (value2 << 4) + (0x0F & value6);
		//	m_pCX5ProcessedData->setTemperature(temperature);
		//	//cout << "Format 2 time full (Package_B): temperature = " << temperature << endl;
		//}
		else if (dataID2 == 0xC0) // timestamp data
		{
			//row_time_stamp[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
			m_iLastRowTimestamp = m_iRowTimestamp;
			m_iRowTimestamp = (value2 << 4) + (0x0F & value6);
			//cout << "Format 2 (Package_B): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimestamp();
		}

		//---------- dataID3[1:0] = data[1:0] = value6[5:4] ----------
		uint8_t dataID3 = 0x30 & value6;
		if (dataID3 == 0x20) //row data
		{
			m_iLastRow = m_iCurrentRow;
			//row_addr[9:0] = data_14_3[13:4] = value3[7:0] + value7[1:0]
			m_iCurrentRow = (value3 << 2) + (0x03 & value7);
			//cout << "Package C: " << "row = " << m_iCurrentRow << endl;
			m_uiRowCount++;
			if (m_iCurrentRow < m_iLastRow)
			{
				//cout << "currentRow = " << m_iCurrentRow << ", lastRow = " << m_iLastRow << endl;
				m_uiEventRowCycleCount++;
			}
			if (m_bEventDenoisingEnabled)
			{
				denoisedPerRow(false);
			}
		}
		else if (dataID3 == 0x10) //col data
		{
			//col_addr[10:0] = data_14_3[13:3] = value3[7:0] + value7[1:0] + value6[7]
			col = (value3 << 3) + ((0x03 & value7) << 1) + ((0x80 & value6) >> 7);
			//cout << "--- Package C: " << "col = " << col << endl;
			saveFormat2Event(col, 0);
		}
		//else if (dataID3 == 0x00) //temperature data
		//{
		//	//temperature[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
		//	temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
		//	m_pCX5ProcessedData->setTemperature(temperature);
		//	//cout << "Format 2 time full (Package_C): temperature = " << temperature << endl;
		//}
		else if (dataID3 == 0x30) //timestamp data
		{
			//row_time_stamp[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
			m_iLastRowTimestamp = m_iRowTimestamp;
			m_iRowTimestamp = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//cout << "Format 2 (Package_C): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimestamp();
		}

		//---------- dataID4[1:0] = data[1:0] = value7[3:2] ----------
		uint8_t dataID4 = 0x0C & value7;
		if (dataID4 == 0x08) //row data
		{
			m_iLastRow = m_iCurrentRow;
			//row_addr[9:0] = data_14_4[13:4] = value4[7:0] + value7[7:6]
			m_iCurrentRow = (value4 << 2) + ((0xC0 & value7) >> 6);
			//cout << "Package D: " << "row = " << m_iCurrentRow << endl;
			m_uiRowCount++;

			if (m_iCurrentRow < m_iLastRow)
			{
				//cout << "currentRow = " << m_iCurrentRow << ", lastRow = " << m_iLastRow << endl;
				m_uiEventRowCycleCount++;
			}
			if (m_bEventDenoisingEnabled)
			{
				denoisedPerRow(false);
			}
		}
		else if (dataID4 == 0x04) //col data
		{
			//col_addr[10:0] = data_14_4[13:3] = value4[7:0] + value7[7:5]
			col = (value4 << 3) + ((0xE0 & value7) >> 5);
			//cout << "--- Package D: " << "col = " << col << endl;
			saveFormat2Event(col, 0);
		}
		//else if (dataID4 == 0x00) //temperature data
		//{
		//	//temperature[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
		//	temperature = (value4 << 4) + ((0xF0 & value7) >> 4);
		//	m_pCX5ProcessedData->setTemperature(temperature);
		//	//cout << "Format 2 time full (Package_D): temperature = " << temperature << endl;
		//}
		else if (dataID4 == 0x0C) //timestamp data
		{
			//row_time_stamp[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
			m_iLastRowTimestamp = m_iRowTimestamp;
			m_iRowTimestamp = (value4 << 4) + ((0xF0 & value7) >> 4);
			//cout << "Format 2 (Package_D): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimestamp();
		}
	}
	m_uiPackageTCounter = 0;

	if (m_bLoopModeEnabled)
	{
		if (dataSize < 357001) //the last package of event data
		{
			m_uiEventFrameNo++;
			if (m_bIMUModuleEnabled)
			{
				parseIMUData(m_lLastPackageTimestamp);
			}
			if (m_bFrameModuleEnabled)
			{
				createImage(timestampEnd);
			}
			else
			{
				m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
			}
			m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);

			m_uiPixelCount = 0;
			m_uiRowCount = 0;
			m_uiEventRowCycleCount = 0;
			m_uiEventTCounter = 0;
			m_vecEventData.clear();
		}
	}

#ifdef _ENABLE_LOG_FILE_
	poll_t = clock();
	m_ofLogFile << "  Elapsed Time: " << (poll_t - begin_t) << " (ms)" << std::endl;
#endif
}

//#define gravity 9.80665
//#define accResolution /*(2 * gravity / (std::pow(2.0,15) -1))*/ 0.000598569
//#define resolution /*(250.0 / (std::pow(2.0, 15)-1) / 180 * CV_PI)*/ 0.000133162
//#define magResolution /*(4800.0 / (std::pow(2.0, 15)-1))*/ 0.146489
const double g_dGravity = 9.80665;
const double g_dAccResolution = 0.000598569;
const double g_dResolution = 0.000133162;
const double g_dMagResolution = 0.146489;
void CeleX5DataProcessor::parseIMUData(std::time_t timestamp)
{
	int end = 0;
	//cout << "-----frame time_stamp-----" << time_stamp << ", size = " << m_vectorIMU_Raw_data.size() << endl;
	for (int i = 0; i < m_vectorIMURawData.size(); i++)
	{
		IMUData imuData;
		memset(&imuData, 0, sizeof(imuData));
		IMURawData imu_raw_data = m_vectorIMURawData[i];
		imuData.frameNo = 0;
		imuData.timestamp = imu_raw_data.timestamp;

		if (imuData.timestamp < timestamp /*&& imuData.time_stamp > time_stamp - 30*/)
		{
			//cout << "imuData.timestamp: " << imuData.timestamp << endl;
			int16_t xACC = (imu_raw_data.imuData[0] << 8) + imu_raw_data.imuData[1];
			int16_t yACC = (imu_raw_data.imuData[2] << 8) + imu_raw_data.imuData[3];
			int16_t zACC = (imu_raw_data.imuData[4] << 8) + imu_raw_data.imuData[5];
			//cout << "xACC = " << xACC << ", yACC = " << yACC << ", zACC = " << zACC << endl;
			imuData.xACC = yACC * g_dAccResolution; //x_ACC * accResolution
			imuData.yACC = xACC * g_dAccResolution; //y_ACC * accResolution
			imuData.zACC = -zACC * g_dAccResolution;
			//cout << "xACC = " << imuData.xACC << ", yACC = " << imuData.yACC << ", zACC = " << imuData.zACC << endl;

			int16_t xTEMP = (imu_raw_data.imuData[6] << 8) + imu_raw_data.imuData[7];
			imuData.xTEMP = xTEMP;
			imuData.xTEMP = 21 + (xTEMP / 333.87);
			//cout << "xTEMP = " << imuData.xTEMP << endl;

			int16_t xGYROS = (imu_raw_data.imuData[8] << 8) + imu_raw_data.imuData[9];
			int16_t yGYROS = (imu_raw_data.imuData[10] << 8) + imu_raw_data.imuData[11];
			int16_t zGYROS = (imu_raw_data.imuData[12] << 8) + imu_raw_data.imuData[13];
			//cout << "xGYROS = " << xGYROS << ", yGYROS = " << yGYROS << ", zGYROS = " << zGYROS << endl;
			imuData.xGYROS = yGYROS * g_dResolution; //xGYROS * resolution
			imuData.yGYROS = -xGYROS * g_dResolution; //yGYROS * resolution
			imuData.zGYROS = zGYROS * g_dResolution;
			//cout << "xGYROS = " << imuData.xGYROS << ", yGYROS = " << imuData.yGYROS << ", zGYROS = " << imuData.zGYROS << endl;

			int16_t xMAG = imu_raw_data.imuData[14] + (imu_raw_data.imuData[15] << 8);
			int16_t yMAG = imu_raw_data.imuData[16] + (imu_raw_data.imuData[17] << 8);
			int16_t zMAG = imu_raw_data.imuData[18] + (imu_raw_data.imuData[19] << 8);
			//cout << "xMAG = " << xMAG << ", yMAG = " << yMAG << ", z_MAG = " << zMAG << endl;
			imuData.xMAG = xMAG * g_dMagResolution;
			imuData.yMAG = yMAG * g_dMagResolution;
			imuData.zMAG = zMAG * g_dMagResolution;
			//cout << "xMAG = " << imuData.xMAG << ", yMAG = " << imuData.yMAG << ", zMAG = " << imuData.zMAG << endl;

			m_vectorIMUData.push_back(imuData);

			//if (m_vectorIMUData.size() > 1000)
			//{
			//	//cout << __FUNCTION__ << ": IMU buffer is full!" << endl;
			//	auto itr = m_vectorIMUData.begin();
			//	m_vectorIMUData.erase(itr);
			//}
			end = i + 1;
		}
	}
	for (int j = 0; j < end; j++)
	{
		auto iter = m_vectorIMURawData.begin();
		m_vectorIMURawData.erase(iter);
	}
}

/*
*  @function: getSensorDataServer
*  @brief   : get CX5SensorDataServer pointer
*  @input   : 
*  @output  :
*  @return  : CX5SensorDataServer pointer
*/
CX5SensorDataServer *CeleX5DataProcessor::getSensorDataServer()
{
	return m_pCX5Server;
}

/*
*  @function: getProcessedData
*  @brief   : get CeleX5ProcessedData pointer
*  @input   :
*  @output  :
*  @return  : CeleX5ProcessedData pointer
*/
CeleX5ProcessedData *CeleX5DataProcessor::getProcessedData()
{
	return m_pCX5ProcessedData;
}

/*
*  @function: checkIfShowImage
*  @brief   : check if the new event pic frame is ready
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::checkIfShowImage()
{
	bool bShowImage = false;
	if (EventShowByRowCycle == m_emEventShowType) //show by row count
	{
		if (m_uiEventRowCycleCount >= m_uiEventRowCycleCountForShow && m_uiRowCount >= 100 * m_uiEventRowCycleCountForShow)
		{
			//cout << "m_uiPixelCount = " << m_uiPixelCount << ", m_uiRowCount = " << m_uiRowCount << endl;
			bShowImage = true;
		}
	}
	else if (EventShowByTime == m_emEventShowType) //show by time
	{
		if (m_uiEventTCounter >= m_uiEventTCountForShow)
		{
			//cout << "m_uiPixelCount = " << m_uiPixelCount << endl;
			bShowImage = true;
		}
	}
	else if (EventShowByCount == m_emEventShowType) //show by event count
	{
		if (m_uiPixelCount >= m_uiEventCountForShow)
		{
			//cout << "m_uiPixelCount = " << m_uiPixelCount << endl;
			bShowImage = true;
		}
	}
	if (bShowImage)
	{
		m_uiEventFrameNo++;

		if (0 == m_lLastPackageTimestamp)
		{
			m_lCurrentEventFrameTimestamp = 0;
		}
		else
		{
			m_lCurrentEventFrameTimestamp = m_lLastPackageTimestamp + m_uiPackageTCounter * 1024 / (m_uiCurrentEventTUnitD * 1000);
		}
		//cout << __FUNCTION__ << ": m_vecEventData.size() = " << m_vecEventData.size() << endl;

		if (m_bIMUModuleEnabled)
		{
			parseIMUData(m_lCurrentEventFrameTimestamp);
		}
		if (m_bFrameModuleEnabled)
		{
			createImage(0);
		}
		else
		{
			m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
		}
		m_uiMinInPixelTimestamp = m_uiMaxInPixelTimestamp;

		if (m_bStartGenerateOFFPN)
		{
			int pixelCount = 0;
			for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
			{
				if (m_pOFFPNEventLatestValue[i] > 0)
				{
					pixelCount++;
				}
			}
			//cout << "-------- " << CELEX5_PIXELS_NUMBER - pixelCount << endl;
			m_pCX5ProcessedData->updateFPNProgress(pixelCount);
			if (pixelCount >= CELEX5_PIXELS_NUMBER-10)
			{
				m_bStartGenerateOFFPN = false;
				FileDirectory base;
				std::string filePath = base.getApplicationDirPath();
#ifdef _WIN32
				filePath += "/FPN_OpticalFlow.txt";
#else
				filePath += "FPN_OpticalFlow.txt";
#endif 
				std::ofstream ofFPNFile;
				ofFPNFile.open(filePath.c_str());
				uint64_t totalValue = 0;
				for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
				{
					totalValue += m_pOFFPNEventLatestValue[i];
				}
				int meanValue = totalValue / CELEX5_PIXELS_NUMBER;
				//cout << "---------- meanValue =  ----------" << meanValue << endl;
				for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
				{
					ofFPNFile << m_pOFFPNEventLatestValue[i] - meanValue << "  ";
					if ((i + 1) % 1280 == 0)
						ofFPNFile << endl;
				}
				ofFPNFile.close();
				memset(m_pOFFPNEventLatestValue, 0, CELEX5_PIXELS_NUMBER * 2);
				cout << "---------- save optical-flow fpn successfully! ----------" << endl;
			}
		}
		//
		m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
		if (m_bEventStreamEnabled)
		{
			m_vecEventData.clear();
		}
		//
		m_uiPixelCount = 0;
		m_uiRowCount = 0;
		m_uiEventRowCycleCount = 0;
		m_uiEventTCounter = 0;
	}
}

/*
*  @function: createImage
*  @brief   : create various frame buffer
*  @input   : time_stamp_end: the pc timestamp when this pacakge data was received from sensor
*  @output  :
*  @return  :
*/
bool CeleX5DataProcessor::createImage(std::time_t timestampEnd)
{
	uint8_t* pFullPic = m_pFullPic[s_uiWriteFrameIndex];
	uint8_t* pEventBinaryPic = m_pEventBinaryPic[s_uiWriteFrameIndex];
	uint8_t* pEventDenoisedBinaryPic = m_pEventDenoisedBinaryPic[s_uiWriteFrameIndex];
	uint8_t* pEventCountPic = m_pEventCountPic[s_uiWriteFrameIndex];
	uint8_t* pEventDenoisedCountPic = m_pEventDenoisedCountPic[s_uiWriteFrameIndex];
	uint8_t* pEventGrayPic = m_pEventGrayPic[s_uiWriteFrameIndex];
	uint8_t* pEventAccumulatedPic = m_pEventAccumulatedPic;
	uint8_t* pEventSuperimposedPic = m_pEventSuperimposedPic[s_uiWriteFrameIndex];
	uint8_t* pEventCountSlicePic = m_pEventCountSlicePic[s_uiWriteFrameIndex];
	uint8_t* pOpticalFlowPic = m_pOpticalFlowPic[s_uiWriteFrameIndex];
	uint8_t* pOpticalFlowSpeedPic = m_pOpticalFlowSpeedPic[s_uiWriteFrameIndex];
	uint8_t* pOpticalFlowDirectionPic = m_pOpticalFlowDirectionPic[s_uiWriteFrameIndex];
	
	m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
	if (m_bLoopModeEnabled)
	{
		//cout << "m_emCurrentSensorMode = " << (int)m_emCurrentSensorMode << endl;
		if (m_emSensorLoopAMode == m_emSensorLoopBMode && m_emSensorLoopBMode == m_emSensorLoopCMode)
		{
			if (m_emCurrentSensorMode == m_emSensorLoopAMode && m_iLastLoopNum == 3)
			{
				m_pCX5ProcessedData->setLoopNum(1);
				m_iLastLoopNum = 1;
			}
			else if (m_emCurrentSensorMode == m_emSensorLoopBMode && m_iLastLoopNum == 1)
			{
				m_pCX5ProcessedData->setLoopNum(2);
				m_iLastLoopNum = 2;
			}
			else if (m_emCurrentSensorMode == m_emSensorLoopCMode && m_iLastLoopNum == 2)
			{
				m_pCX5ProcessedData->setLoopNum(3);
				m_iLastLoopNum = 3;
			}
		}
		else if (m_emSensorLoopAMode == m_emSensorLoopBMode || m_emSensorLoopAMode == m_emSensorLoopCMode || m_emSensorLoopBMode == m_emSensorLoopCMode)
		{
			CeleX5::CeleX5Mode diffMode;
			int diff_loop_num = -1;
			if (m_emSensorLoopAMode == m_emSensorLoopBMode)
			{
				diffMode = m_emSensorLoopCMode;
				diff_loop_num = 3;
			}
			else if (m_emSensorLoopAMode == m_emSensorLoopCMode)
			{
				diffMode = m_emSensorLoopBMode;
				diff_loop_num = 2;
			}
			else
			{
				diffMode = m_emSensorLoopAMode;
				diff_loop_num = 1;
			}
			if (m_emCurrentSensorMode == diffMode)
			{
				m_pCX5ProcessedData->setLoopNum(diff_loop_num);
				m_iLastLoopNum = diff_loop_num;
			}
			else if (m_iLastLoopNum == diff_loop_num)
			{
				int same_loop_num = diff_loop_num + 1;
				if (same_loop_num > 3)
					same_loop_num = 1;
				m_pCX5ProcessedData->setLoopNum(same_loop_num);
				m_iLastLoopNum = same_loop_num;
			}
			else
			{
				int same_loop_num = m_iLastLoopNum + 1;
				if (same_loop_num > 3)
					same_loop_num = 1;
				m_pCX5ProcessedData->setLoopNum(same_loop_num);
				m_iLastLoopNum = same_loop_num;
			}
		}
		else
		{
			if (m_emCurrentSensorMode == m_emSensorLoopAMode)
			{
				m_pCX5ProcessedData->setLoopNum(1);
			}
			else if (m_emCurrentSensorMode == m_emSensorLoopBMode)
			{
				m_pCX5ProcessedData->setLoopNum(2);
			}
			else if (m_emCurrentSensorMode == m_emSensorLoopCMode)
			{
				m_pCX5ProcessedData->setLoopNum(3);
			}
		}
	}
	else
	{
		m_pCX5ProcessedData->setLoopNum(-1);
	}

	int countValue = 0, adcValue = 0, index = 0;
	int row, col, index1, index2, index3, index4;
	if (m_emCurrentSensorMode == CeleX5::Event_Off_Pixel_Timestamp_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);
			countValue = m_pEventCountBuffer[i];
			if (countValue > 0)
			{
				countValue = countValue * m_uiEventCountStep;
				countValue = countValue > 255 ? 255 : countValue;
				pEventBinaryPic[index] = 255;
				pEventDenoisedBinaryPic[index] = 255;
				//
				pEventCountPic[index] = countValue;
				pEventDenoisedCountPic[index] = countValue;
			}
			else
			{
				pEventBinaryPic[index] = 0;
				pEventDenoisedBinaryPic[index] = 0;
				//
				pEventCountPic[index] = 0;
				pEventDenoisedCountPic[index] = 0;
			}
		}
		if (m_bFrameDenoisingEnabled)
		{
			//--- calculate denoised pic ---
			calculateDenoisedBuffer(pEventDenoisedBinaryPic, pEventDenoisedCountPic, pEventCountPic, 4);
		}
		if (m_bEventCountDensityEnabled)
		{
			//--- calculate event count slice ---
			calEventCountSlice(pEventCountSlicePic);
		}
		m_lEventFrameTimestamp = timestampEnd;
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_In_Pixel_Timestamp_Mode)
	{
		int len = m_uiMaxInPixelTimestamp - m_uiMinInPixelTimestamp;
		if (len <= 0)
			len = 1;
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);
			adcValue = m_pEventADCBuffer[i];
			if (adcValue > 0)
			{
				countValue = m_pEventCountBuffer[i] * m_uiEventCountStep;
				adcValue = adcValue - m_uiMinInPixelTimestamp;
				if (adcValue < 0)
					adcValue = 0;
				adcValue = adcValue * 265 / len;
				if (adcValue > 255)
					adcValue = 255;
				pOpticalFlowPic[index] = adcValue;
				///
				pEventBinaryPic[index] = 255;
				pEventDenoisedBinaryPic[index] = 255;
				//
				pEventCountPic[index] = countValue;
				pEventDenoisedCountPic[index] = countValue;
			}
			else
			{
				pOpticalFlowPic[index] = 0;
				//
				pEventBinaryPic[index] = 0;
				pEventDenoisedBinaryPic[index] = 0;
				//
				pEventCountPic[index] = 0;
				pEventDenoisedCountPic[index] = 0;
			}
		}
		if (m_bFrameDenoisingEnabled)
		{
			//--- calculate denoised pic ---
			calculateDenoisedBuffer(pEventDenoisedBinaryPic, pEventDenoisedCountPic, pEventCountPic, 4);
		}
		if (m_bEventOpticalFlowEnabled)
		{
			//--- calculate optical-flow direction and speed ---
			calDirectionAndSpeed(m_pEventADCBuffer, pOpticalFlowSpeedPic, pOpticalFlowDirectionPic);
		}
		if (m_bEventCountDensityEnabled)
		{
			//--- calculate event count slice ---
			calEventCountSlice(pEventCountSlicePic);
		}
		m_lEventFrameTimestamp = timestampEnd;
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Intensity_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);
			adcValue = m_pEventADCBuffer[i];
			if (adcValue > 0)
			{
				countValue = m_pEventCountBuffer[i] * m_uiEventCountStep;
				adcValue = adcValue - m_pFpnBuffer[i];
				if (adcValue < 0)
					adcValue = 0;
				if (adcValue > 255)
					adcValue = 255;
				pEventBinaryPic[index] = 255;
				pEventGrayPic[index] = adcValue;
				pEventAccumulatedPic[index] = adcValue; //don't need to clear
				pEventSuperimposedPic[index] = pEventBinaryPic[index] > 0 ? 255 : adcValue;//superimpose
				//
				pEventCountPic[index] = countValue;
			}
			else
			{
				pEventBinaryPic[index] = 0;
				pEventGrayPic[index] = 0;
				pEventSuperimposedPic[index] = pEventAccumulatedPic[index];
				//
				pEventCountPic[index] = 0;
			}
		}
		if (m_bEventCountDensityEnabled)
		{
			calEventCountSlice(pEventCountSlicePic);
		}
		m_lEventFrameTimestamp = timestampEnd;
	}
	else if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode)
	{
		bool bAdjust = false;
		if (m_bLoopModeEnabled)
		{
			if ((int)m_emLastLoopMode < 3)
			{
				if (m_pEventADCBuffer[m_iFPNIndexForAdjustPic] > m_pEventADCBuffer[m_iFPNIndexForAdjustPic + 1])
				{
					bAdjust = true;
				}
			}
		}
		for (int i = 1; i < CELEX5_PIXELS_NUMBER; i++)
		{
			row = i / CELEX5_COL;
			col = i % CELEX5_COL;
			index = getCurrentIndex(i);
			if (row == 0 || row == 799 || col == 0 || col == 1279)
				continue;
			index1 = (row - 1)*CELEX5_COL + col;
			index2 = row*CELEX5_COL + col - 1;
			index3 = row*CELEX5_COL + col + 1;
			index4 = (row + 1)*CELEX5_COL + col;

			if (bAdjust)
			{
				adcValue = m_pEventADCBuffer[i] - m_pFpnBuffer[i-1];//Subtract FPN
				if (adcValue < 0)
					adcValue = 0;
				if (adcValue > 255)
					adcValue = 255;
				pFullPic[index-1] = adcValue;
			}
			else
			{
				if (bAdjust)
				{
					if (m_pEventADCBuffer[i] < 255 && m_pEventADCBuffer[i] > 0)
						adcValue = m_pEventADCBuffer[i] - m_pFpnBuffer[i-1];//Subtract FPN
					else if (m_pEventADCBuffer[index1] < 255 && m_pEventADCBuffer[index1] > 0)
						adcValue = m_pEventADCBuffer[index1] - m_pFpnBuffer[index1-1];
					else if (m_pEventADCBuffer[index2] < 255 && m_pEventADCBuffer[index2] > 0)
						adcValue = m_pEventADCBuffer[index2] - m_pFpnBuffer[index2-1];
					else if (m_pEventADCBuffer[index3] < 255 && m_pEventADCBuffer[index3] > 0)
						adcValue = m_pEventADCBuffer[index3] - m_pFpnBuffer[index3-1];
					else if (m_pEventADCBuffer[index4] < 255 && m_pEventADCBuffer[index4] > 0)
						adcValue = m_pEventADCBuffer[index4] - m_pFpnBuffer[index4-1];
					else
						adcValue = m_pEventADCBuffer[i];
				}
				else
				{
					if (m_pEventADCBuffer[i] < 255 && m_pEventADCBuffer[i] > 0)
						adcValue = m_pEventADCBuffer[i] - m_pFpnBuffer[i];//Subtract FPN
					else if (m_pEventADCBuffer[index1] < 255 && m_pEventADCBuffer[index1] > 0)
						adcValue = m_pEventADCBuffer[index1] - m_pFpnBuffer[index1];
					else if (m_pEventADCBuffer[index2] < 255 && m_pEventADCBuffer[index2] > 0)
						adcValue = m_pEventADCBuffer[index2] - m_pFpnBuffer[index2];
					else if (m_pEventADCBuffer[index3] < 255 && m_pEventADCBuffer[index3] > 0)
						adcValue = m_pEventADCBuffer[index3] - m_pFpnBuffer[index3];
					else if (m_pEventADCBuffer[index4] < 255 && m_pEventADCBuffer[index4] > 0)
						adcValue = m_pEventADCBuffer[index4] - m_pFpnBuffer[index4];
					else
						adcValue = m_pEventADCBuffer[i];
				}
				if (adcValue < 0)
					adcValue = 0;
				if (adcValue > 255)
					adcValue = 255;

				if (bAdjust)
					pFullPic[index - 1] = adcValue;
				else
					pFullPic[index] = adcValue;
			}
		}
		m_lFullFrameTimestamp = timestampEnd;
	}
	else if (m_emCurrentSensorMode == CeleX5::Optical_Flow_Mode ||
		     m_emCurrentSensorMode == CeleX5::Multi_Read_Optical_Flow_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);
			adcValue = m_pEventADCBuffer[i];
			//cout << "m_pEventADCBuffer: " << value << endl;
			if (adcValue == 255)
				adcValue = 0;
			pOpticalFlowPic[index] = adcValue;
			calDirectionAndSpeed(i, index, m_pEventADCBuffer, pOpticalFlowSpeedPic, pOpticalFlowDirectionPic);
		}
		m_lOpticalFrameTimestamp = timestampEnd;
	}
	else if (m_emCurrentSensorMode == CeleX5::Optical_Flow_FPN_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);
			adcValue = m_pEventADCBuffer[i] >> 4;
			//cout << "m_pEventADCBuffer: " << value << endl;
			if (adcValue == 255)
				adcValue = 0;
			pOpticalFlowPic[index] = adcValue;
		}
	}
	//
	if (m_bLoopModeEnabled)
	{
		m_emLastLoopMode = m_emCurrentSensorMode;
	}
	if (m_bEventCountDensityEnabled)
	{
		if (m_uiEventCountSliceNum == 5)
		{
			m_pEventCountSlice[3] = m_pEventCountSlice[2];
			m_pEventCountSlice[2] = m_pEventCountSlice[1];
			m_pEventCountSlice[1] = m_pEventCountSlice[0];
			m_pEventCountSlice[0] = m_pEventCountSlice[4];
		}
		else if (m_uiEventCountSliceNum == 6)
		{
			m_pEventCountSlice[4] = m_pEventCountSlice[3];
			m_pEventCountSlice[3] = m_pEventCountSlice[2];
			m_pEventCountSlice[2] = m_pEventCountSlice[1];
			m_pEventCountSlice[1] = m_pEventCountSlice[0];
			m_pEventCountSlice[0] = m_pEventCountSlice[5];
		}
		else if (m_uiEventCountSliceNum == 7)
		{
			m_pEventCountSlice[5] = m_pEventCountSlice[4];
			m_pEventCountSlice[4] = m_pEventCountSlice[3];
			m_pEventCountSlice[3] = m_pEventCountSlice[2];
			m_pEventCountSlice[2] = m_pEventCountSlice[1];
			m_pEventCountSlice[1] = m_pEventCountSlice[0];
			m_pEventCountSlice[0] = m_pEventCountSlice[6];
		}
		else if (m_uiEventCountSliceNum == 8)
		{
			m_pEventCountSlice[6] = m_pEventCountSlice[5];
			m_pEventCountSlice[5] = m_pEventCountSlice[4];
			m_pEventCountSlice[4] = m_pEventCountSlice[3];
			m_pEventCountSlice[3] = m_pEventCountSlice[2];
			m_pEventCountSlice[2] = m_pEventCountSlice[1];
			m_pEventCountSlice[1] = m_pEventCountSlice[0];
			m_pEventCountSlice[0] = m_pEventCountSlice[7];
		}
		memcpy(m_pEventCountSlice[0], m_pEventCountBuffer, CELEX5_PIXELS_NUMBER);
	}
	memset(m_pEventCountBuffer, 0, CELEX5_PIXELS_NUMBER);
	memset(m_pEventADCBuffer, 0, CELEX5_PIXELS_NUMBER*4);

	if (s_uiWriteFrameIndex == 0)
	{
		s_uiReadFrameIndex = 0;
		s_uiWriteFrameIndex = 1;
	}
	else
	{
		s_uiReadFrameIndex = 1;
		s_uiWriteFrameIndex = 0;
	}
	return true;
}

/*
*  @function: generateFPNimpl
*  @brief   : 
*  @input   : 
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::generateFPNimpl()
{
	for (int i = 0; i < CELEX5_PIXELS_NUMBER; ++i)
	{
		if (m_iFpnCalculationTimes == FPN_CALCULATION_TIMES)
		{
			m_pFpnGenerationBuffer[i] = m_pEventADCBuffer[i];
		}
		else
		{
			m_pFpnGenerationBuffer[i] += m_pEventADCBuffer[i];
		}
	}
	--m_iFpnCalculationTimes;

	if (m_iFpnCalculationTimes <= 0)
	{
		m_bIsGeneratingFPN = false;
		std::ofstream ff;
		FileDirectory base;
		std::string filePath = base.getApplicationDirPath();
		if (m_emCurrentSensorMode == CeleX5::Optical_Flow_FPN_Mode)
		{	
#ifdef _WIN32
			filePath += "/FPN_OpticalFlow.txt";
#else
			filePath += "FPN_OpticalFlow.txt";
#endif 
			// output the FPN file now
			ff.open(filePath.c_str());
		}
		else
		{
			if (m_strFpnFilePath.empty())
			{
#ifdef _WIN32
				filePath += "/FPN_";
#else
				filePath += "FPN_";
#endif 
				std::stringstream level;
				level << m_uiISOLevel;
				//
				filePath += string(level.str());
				filePath += ".txt";

				// output the FPN file now
				ff.open(filePath.c_str());
			}
			else
			{
				ff.open(m_strFpnFilePath.c_str());
			}
		}
		if (!ff)
			return;
		uint64_t total = 0;
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; ++i)
		{
			m_pFpnGenerationBuffer[i] = m_pFpnGenerationBuffer[i] / FPN_CALCULATION_TIMES;
			total += m_pFpnGenerationBuffer[i];
		}
		//cout << "total = " << total << endl;
		int avg = total / CELEX5_PIXELS_NUMBER;
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; ++i)
		{
			int d = m_pFpnGenerationBuffer[i] - avg;
			ff << d << "  ";			
			//cout << avg << endl;

			if ((i + 1) % 1280 == 0)
				ff << "\n";
		}
		ff.close();
	}
}

void CeleX5DataProcessor::loadOpticalFlowFPN()
{
	FileDirectory fileDir;
	std::string filePath = fileDir.getApplicationDirPath();
#ifdef _WIN32
	filePath += "/FPN_OpticalFlow.txt";
#else
	filePath += "FPN_OpticalFlow.txt";
#endif
	int index = 0;
	std::ifstream in;
	in.open(filePath.c_str());
	if (in.is_open())
	{
		std::string line;
		while (!in.eof() && index < CELEX5_PIXELS_NUMBER)
		{
			in >> line;
			m_pFpnBufferOF[index] = atoi(line.c_str());
			//cout << index << ", " << m_pFpnBuffer_OF[index] << endl;
			index++;
		}
		cout << "Load Optical-flow fpn successfully!" << endl;
		in.close();
	}
}

/*
*  @function: setFpnFile
*  @brief   : set the 
*  @input   : fpnFile: fpn file path and name
*  @output  :
*  @return  :
*/
bool CeleX5DataProcessor::setFpnFile(const std::string& fpnFile)
{
	cout << __FUNCTION__ << ": fpnFile = " << fpnFile << endl;
	int index = 0;
	std::ifstream in;
	in.open(fpnFile.c_str());
	if (!in.is_open())
	{
		return false;
	}
	std::string line;
	while (!in.eof() && index < CELEX5_PIXELS_NUMBER)
	{
		in >> line;
		m_pFpnBuffer[index] = atoi(line.c_str());
		//cout << index << ", " << m_pFpnBuffer[index] << endl;
		index++;
	}
	if (index != CELEX5_PIXELS_NUMBER)
		return false;
	//cout << "fpn count = " << index << endl;
	in.close();
	cout << __FUNCTION__ << ": set fpn successfully!" << endl;

	for (int j = 1; j < 1000000; j++)
	{
		if (m_pFpnBuffer[j+1] - m_pFpnBuffer[j] > 5 && m_pFpnBuffer[j+1] - m_pFpnBuffer[j+2] > 5)
		{
			m_iFPNIndexForAdjustPic = j;
			cout << "m_iFPNIndexForAdjustPic = " << m_iFPNIndexForAdjustPic << endl;
			break;
		}
	}
	return true;
}

/*
*  @function: generateFPN
*  @brief   : generate a FPN file
*  @input   : fpnFile: the directory path and name of the fpn file to be generated
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::generateFPN(std::string filePath)
{
	if (getSensorFixedMode() == CeleX5::Event_In_Pixel_Timestamp_Mode)
	{
		m_bStartGenerateOFFPN = true;
	}
	else
	{
		m_bIsGeneratingFPN = true;
		m_iFpnCalculationTimes = FPN_CALCULATION_TIMES;
		m_strFpnFilePath = filePath;
	}
}

/*
*  @function: getSensorFixedMode
*  @brief   : get sensor fixed mode
*  @input   : 
*  @output  :
*  @return  :
*/
CeleX5::CeleX5Mode CeleX5DataProcessor::getSensorFixedMode()
{
	return m_emSensorFixedMode;
}

/*
*  @function: getSensorLoopMode
*  @brief   : get the mode of one of three loops according to the specific loop number
*  @input   : loopNum: loop number (1/2/3)
*  @output  :
*  @return  :
*/
CeleX5::CeleX5Mode CeleX5DataProcessor::getSensorLoopMode(int loopNum)
{
	if (1 == loopNum)
		return m_emSensorLoopAMode;
	else if (2 == loopNum)
		return m_emSensorLoopBMode;
	else if (3 == loopNum)
		return m_emSensorLoopCMode;
	else
		return CeleX5::Unknown_Mode;
}

/*
*  @function: setSensorFixedMode
*  @brief   : set sensor fixed mode
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setSensorFixedMode(CeleX5::CeleX5Mode mode)
{
	m_emCurrentSensorMode = m_emSensorFixedMode = mode;
	m_uiEventFrameNo = 0;
}

/*
*  @function: setSensorLoopMode
*  @brief   : set the mode of one of three loops according to the specific loop number
*  @input   : mode: CeleX-5 sensor mode
*             loopNum: loop number (1/2/3)
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setSensorLoopMode(CeleX5::CeleX5Mode mode, int loopNum)
{
	if (1 == loopNum)
		m_emSensorLoopAMode = mode;
	else if (2 == loopNum)
		m_emSensorLoopBMode = mode;
	else if (3 == loopNum)
		m_emSensorLoopCMode = mode;
}

/*
*  @function: setLoopModeEnabled
*  @brief   : enable the loop mode of the CeleX-5 sensor
*  @input   : enable: the state to enable or disable Loop Mode
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setLoopModeEnabled(bool enable)
{
	m_bLoopModeEnabled = enable;
	m_uiEventFrameNo = 0;
}

/*
*  @function: setISOLevel
*  @brief   : set ISO level
*  @input   : value: ISO level (1 ~ 4)
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setISOLevel(uint32_t value)
{
	m_uiISOLevel = value;
	//--- load default fpn file ---
	FileDirectory base;
	std::string filePath = base.getApplicationDirPath();
#ifdef _WIN32
	filePath += "/FPN_";
#else
	filePath += "FPN_";
#endif
	std::stringstream level;
	level << m_uiISOLevel;
	filePath += string(level.str());
	filePath += ".txt";
	setFpnFile(filePath);
}

/*
*  @function: setISOLevel
*  @brief   : set ISO level
*  @input   : value: ISO level (1 ~ 6)
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setMIPIDataFormat(int format)
{
	m_iMIPIDataFormat = format;
	cout << __FUNCTION__ << ": m_iMIPIDataFormat = " << m_iMIPIDataFormat << endl;
}

/*
*  @function: setEventFrameTime
*  @brief   : set event frame time duration
*  @input   : value: event frame time duration (unit: microsecond)
*             clock: crrent clock rate
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setEventFrameTime(uint32_t value, uint32_t clock) //unit: microsecond
{
	m_uiEventFrameTime = value;
	m_uiCurrentEventTUnitD = m_uiEventTUnitDList[clock / 10 - 1];
	m_uiEventTCountForShow = value * m_uiCurrentEventTUnitD / m_uiEventTUnitN;
	m_uiEventTCountForEPS = 1000000 * m_uiCurrentEventTUnitD / m_uiEventTUnitN;
	cout << __FUNCTION__ << ": m_uiEventTCountForShow = " << m_uiEventTCountForShow << endl;
	cout << __FUNCTION__ << ": m_uiEventTCountForEPS = " << m_uiEventTCountForEPS << endl;
}

/*
*  @function: getEventFrameTime
*  @brief   : get event frame time duration
*  @input   : 
*  @output  :
*  @return  :
*/
uint32_t CeleX5DataProcessor::getEventFrameTime()
{
	return m_uiEventFrameTime;
}

void CeleX5DataProcessor::setEventCountSliceNum(uint32_t value)
{
	m_uiEventCountSliceNum = value;
	cout << "CeleX5DataProcessor::setEventCountSliceNum = " << value << endl;
}

uint32_t CeleX5DataProcessor::getEventCountSliceNum()
{
	return m_uiEventCountSliceNum;
}

/*
*  @function: setEventShowMethod
*  @brief   : 
*  @input   : type:
*             value: 
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setEventShowMethod(EventShowType type, int value)
{
	m_emEventShowType = type;
	if (EventShowByTime == type)
	{
		m_uiEventTCountForShow = value;
	}
	else if (EventShowByRowCycle == type)
	{
		m_uiEventRowCycleCountForShow = value;
	}
	else if (EventShowByCount == type)
	{
		m_uiEventCountForShow = value;
	}
}

/*
*  @function: getEventShowMethod
*  @brief   : get the method of creating event frame (by time, count or row circle)
*  @input   :
*  @output  :
*  @return  :
*/
EventShowType CeleX5DataProcessor::getEventShowMethod()
{
	return m_emEventShowType;
}

/*
*  @function : setEventFrameStartPos
*  @brief    : set the start pos of the event data in the loop mode
*  @input    : value : start position
*  @output   :
*  @return   :
*/
void CeleX5DataProcessor::setEventFrameStartPos(uint32_t value)
{
	m_uiEventTCountForRemove = value * m_uiEventTCountForShow * 1000 / m_uiEventFrameTime;
	cout << __FUNCTION__ << ": m_uiEventTCountForRemove = " << m_uiEventTCountForRemove << endl;
}

void CeleX5DataProcessor::calculateDenoisedBuffer(uint8_t* pDesBuffer1, uint8_t* pDesBuffer2, uint8_t* pSrcBuffer, int neighbours)//4 neighbours
{
	if (nullptr == pDesBuffer1 || nullptr == pDesBuffer2 || nullptr == pSrcBuffer)
	{
		return;
	}
	if (4 == neighbours)
	{
		for (int pos = 0; pos < CELEX5_PIXELS_NUMBER; pos++)
		{
			if (pSrcBuffer[pos] > 0)
			{
				//int row = pos / CELEX5_COL;
				if (pos < 1280 || pos > 1022720)
				{
					pDesBuffer1[pos] = 0;
					pDesBuffer2[pos] = 0;
					continue;
				}
				int col = pos % CELEX5_COL;
				if (col == 0 || col == 1279)
				{
					pDesBuffer1[pos] = 0;
					pDesBuffer2[pos] = 0;
					continue;
				}
				int eventCount = 0;

				if (pSrcBuffer[pos - 1280] > 0)
					++eventCount;
				if (pSrcBuffer[pos + 1280] > 0)
					++eventCount;
				if (pSrcBuffer[pos - 1] > 0)
					++eventCount;
				if (pSrcBuffer[pos + 1] > 0)
					++eventCount;

				if (eventCount < 4)
				{
					pDesBuffer1[pos] = 0;
					pDesBuffer2[pos] = 0;
				}
			}
		}
	}
	else if (8 == neighbours)
	{
		for (int pos = 0; pos < CELEX5_PIXELS_NUMBER; pos++)
		{
			if (pSrcBuffer[pos] > 0)
			{
				//int row = pos / CELEX5_COL;
				if (pos < 1280 || pos > 1022720)
				{
					pDesBuffer1[pos] = 0;
					pDesBuffer2[pos] = 0;
					continue;
				}
				int col = pos % CELEX5_COL;
				if (col == 0 || col == 1279)
				{
					pDesBuffer1[pos] = 0;
					pDesBuffer2[pos] = 0;
					continue;
				}
				int eventCount = 0;

				if (pSrcBuffer[pos - 1280] > 0)
					++eventCount;
				if (pSrcBuffer[pos - 1279] > 0)
					++eventCount;
				if (pSrcBuffer[pos + 1280] > 0)
					++eventCount;
				if (pSrcBuffer[pos + 1281] > 0)
					++eventCount;
				if (pSrcBuffer[pos - 1] > 0)
					++eventCount;
				if (pSrcBuffer[pos - 2] > 0)
					++eventCount;
				if (pSrcBuffer[pos + 1] > 0)
					++eventCount;
				if (pSrcBuffer[pos + 2] > 0)
					++eventCount;

				if (eventCount < 2)
				{
					pDesBuffer1[pos] = 0;
					pDesBuffer2[pos] = 0;
				}
			}
		}
	}
}

void CeleX5DataProcessor::calDirectionAndSpeed(int i, int j, uint32_t* pBuffer, uint8_t* &speedBuffer, uint8_t* &dirBuffer)
{
	int row = i / CELEX5_COL;
	int col = i % CELEX5_COL;
	int Gx = 0, Gy = 0;

	if (col == 0 || col == CELEX5_COL - 1)
		Gx = 0;
	else
		Gx = pBuffer[i + 1] - pBuffer[i - 1];

	if (row == 0 || row == CELEX5_ROW - 1)
		Gy = 0;
	else
		Gy = pBuffer[i + CELEX5_COL] - pBuffer[i - CELEX5_COL];

	int theta = 0;
	if (Gx == 0 && Gy == 0)
	{
		theta = 0;
	}
	else
	{
		if (Gx == 0)
		{
			if (Gy > 0)
				theta = 90;
			else
				theta = 270;
		}
		else
		{
			theta = atan2(Gy, Gx) * 180 / CV_PI;
		}
	}
	if (theta < 0)
		theta += 360;
	dirBuffer[j] = theta * 255 / 360;

	int value1 = sqrt(Gx*Gx + Gy*Gy);
	if (value1 > 255)
		value1 = 255;
	speedBuffer[j] = value1;
}

void CeleX5DataProcessor::calDirectionAndSpeed(uint32_t* pBuffer, uint8_t* &speedBuffer, uint8_t* &dirBuffer)
{
	//int index = 0;
	int markH = 2;
	int markV = CELEX5_COL * 2;
	for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
	{
		//index = getCurrentIndex(i);
		if (pBuffer[i] > 0)
		{
			int row = i / CELEX5_COL;
			int col = i % CELEX5_COL;
			int Gx = 0, Gy = 0;

			if (col <= 200 || col >= CELEX5_COL - 200)
			{
				dirBuffer[i] = 0;
				//speedBuffer[i] = 0;
				continue;
			}
			if (row <= 150 || row >= CELEX5_ROW - 150)
			{
				dirBuffer[i] = 0;
				//speedBuffer[i] = 0;
				continue;
			}
			if (pBuffer[i + markH] == 0 || pBuffer[i - markH] == 0 || pBuffer[i + markV] == 0 || pBuffer[i - markV] == 0)
			{
				dirBuffer[i] = 0;
				//speedBuffer[i] = 0;
				continue;
			}
			Gx = pBuffer[i + markH] - pBuffer[i - markH];
			Gy = pBuffer[i + markV] - pBuffer[i - markV];

			//cout << "Gx = " << Gx << ", Gy = " << Gy << endl;

			int theta = 0;
			if (abs(Gy) - abs(Gx) > 50)
			{
				if (Gy > 10)
					theta = 200;
				else if (Gy < -10)
					theta = 90;
			}
			else if (abs(Gx) - abs(Gy) > 50)
			{
				if (Gx > 50)
					theta = 120;
				else if (Gx < -50)
					theta = 10;
			}
			dirBuffer[i] = theta;
		}
		else
		{
			dirBuffer[i] = 0;
			//speedBuffer[i] = 0;
		}
	}
}

/*
*  @function: setRotateType
*  @brief   : 
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setRotateType(int type)
{
	m_iRotateType = type;
}

/*
*  @function: getRotateType
*  @brief   : 
*  @input   :
*  @output  :
*  @return  :
*/
int CeleX5DataProcessor::getRotateType()
{
	return m_iRotateType;
}

/*
*  @function: setEventCountStep
*  @brief   :
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::setEventCountStep(uint32_t step)
{
	m_uiEventCountStep = step;
}

/*
*  @function: getEventCountStep
*  @brief   :
*  @input   :
*  @output  :
*  @return  :
*/
uint32_t CeleX5DataProcessor::getEventCountStep()
{
	return m_uiEventCountStep;
}

/*
*  @function: getIMUData
*  @brief   :
*  @input   :
*  @output  :
*  @return  :
*/
int CeleX5DataProcessor::getIMUData(std::vector<IMUData>& data)
{
	data = m_vectorIMUData;
	m_vectorIMUData.clear();
	return data.size();
}

/*
*  @function: saveFullPicRawData
*  @brief   : 
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::saveFullPicRawData()
{
	//cout << "----- CeleX5DataProcessor::saveFullPicRawData" << endl;
	m_bSaveFullPicRawData = true;
}

/*
*  @function: resetTimestamp
*  @brief   : reset all of the data buffer and related parameters
*  @input   : 
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::resetTimestamp()
{
	m_vecEventData.clear();
	m_vectorIMUData.clear();
	m_vectorIMURawData.clear();

	m_iLastRowTimestamp = -1;
	m_iRowTimestamp = -1;

	m_iLastRow = -1;
	m_iCurrentRow = -1;
	m_uiRowCount = 0;

	m_uiEventTCounter = 0;
	m_uiEventTCounterTotal = 0;

	m_uiEventNumberEPS = 0;
	m_uiPixelCountForEPS = 0;
	m_uiEventTCounterEPS = 0;

	m_uiEOTrampNo = 1;

	m_uiMinInPixelTimestamp = 0;
	m_uiMaxInPixelTimestamp = 0;
}

/*
*  @function: getEventRate
*  @brief   : get the number of events fired per second  
*  @input   :
*  @output  :
*  @return  : the number of events fired per second
*/
uint32_t CeleX5DataProcessor::getEventRate()
{
	return m_uiEventNumberEPS;
}

uint32_t CeleX5DataProcessor::getEventRatePerFrame()
{
	return m_uiPixelCount;
}

/*
*  @function: saveFullPicRawData
*  @brief   : convert a full-frame pic buffer to a txt named "full_pic_raw_data.txt"
*  @input   : pData: full-frame pic data pointer
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::saveFullPicRawData(uint8_t* pData)
{
	//cout << "CeleX5DataProcessor::saveFullPicRawData" << endl;
	ofstream ofFullPic;
	std::string fileName = "full_pic_raw_data_";

	std::stringstream fpnNo;
	int fpnCount = 1;
	fpnNo << fpnCount;
	fileName += string(fpnNo.str());
	fileName += ".txt";
	ofFullPic.open(fileName);

	int dataSize = 1536001;
	int index = 0;
	for (int i = 0; i < dataSize - 2; i += 3)
	{
		uint8_t value1 = *(pData + i);
		uint8_t value2 = *(pData + i + 1);
		uint8_t value3 = *(pData + i + 2);

		uint16_t adc11 = (value1 << 4) + (0x0F & value3);
		uint16_t adc22 = (value2 << 4) + ((0xF0 & value3) >> 4);

		ofFullPic << (4095 - adc11) << "  " << (4095 - adc22) << "  "; 
		//ofFullPic << 255 - (int)value1 << "  " << 255 - (int)value2 << "  ";
		index += 2;
		if (index % 1280 == 0)
			ofFullPic << endl;
	}
	ofFullPic.close();
	m_bSaveFullPicRawData = false;
}

/*
*  @function : findModeInLoopGroup
*  @brief    : check if the specific mode is in the loop group
*  @input    : mode: sensor mode
*  @output   :
*  @return   :
*/
bool CeleX5DataProcessor::findModeInLoopGroup(CeleX5::CeleX5Mode mode)
{
	if (mode == m_emSensorLoopAMode || mode == m_emSensorLoopBMode || mode == m_emSensorLoopCMode)
		return true;
	else
		return false;
}

/*
*  @function: processMIPIEventTimestamp
*  @brief   : process MIPI event timestamp
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::processMIPIEventTimestamp()
{
	//cout << "m_iRowTimeStamp = " << m_iRowTimeStamp << ", m_iLastRowTimeStamp = " << m_iLastRowTimeStamp << endl;
	int diffT = m_iRowTimestamp - m_iLastRowTimestamp;
	if (diffT < 0)
	{
		if (1 == m_iMIPIDataFormat)
			diffT += g_uiMPDataFormat1MaxLen;
		else
			diffT += g_uiMPDataFormat2MaxLen;
	}
	/*if (diffT > 1)
	cout << __FUNCTION__ << ": T is not continuous!" << endl;*/
	if (m_iLastRowTimestamp != -1/* && diffT < 5*/)
	{
		m_uiEventTCounter += diffT;
		m_uiEventTCounterTotal += diffT;
		m_uiEventTCounterEPS += diffT;
		m_uiPackageTCounter += diffT;
		//cout << "m_uiEventTCounter_Total = " << m_uiEventTCounter_Total << endl;
	}

	if (m_emCurrentSensorMode == CeleX5::Event_In_Pixel_Timestamp_Mode)
	{
		if (m_iRowTimestamp % 1024 == 0 && diffT != 0)
		{
			m_uiEOTrampNo++;
			//cout << "m_uiEOTrampNo = " << m_uiEOTrampNo << ", m_iRowTimeStamp = "<< m_iRowTimeStamp << endl;
		}
	}
	if (!m_bLoopModeEnabled)
	{
		checkIfShowImage();
	}
	if (m_uiEventTCounterEPS > m_uiEventTCountForEPS)
	{
		//cout << "m_uiPixelCountForEPS = " << m_uiPixelCountForEPS << endl;
		m_uiEventNumberEPS = m_uiPixelCountForEPS;
		m_uiPixelCountForEPS = 0;
		m_uiEventTCounterEPS = 0;
	}
}

/*
*  @function: saveIntensityEvent
*  @brief   : 
*  @input   : col:
*             adc12bit:
*             adc8bit: 
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::saveIntensityEvent(int col, int adc12bit, int adc8bit)
{
	int index = m_iCurrentRow * CELEX5_COL + col;
	if (!m_bEventDenoisingEnabled)
	{
		/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
		m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
		m_pEventCountBuffer[index] += 1;
		m_pEventADCBuffer[index] = adc8bit;
		m_uiPixelCount++;
		m_uiPixelCountForEPS++;
	}
	if (m_bEventStreamEnabled)
	{
		EventData eventData;
		eventData.row = m_iCurrentRow;
		eventData.col = col;
		eventData.adc = adc12bit;
		//eventData.t_off_pixel = m_uiEventTCounter;
		eventData.tOffPixelIncreasing = m_uiEventTCounterTotal * m_uiEventTUnitN / m_uiCurrentEventTUnitD;
		eventData.tInPixelIncreasing = 0;

		//--- cal polarity ---
		if (adc12bit > m_pLastADC[index])
			eventData.polarity = 1;
		else if (adc12bit < m_pLastADC[index])
			eventData.polarity = -1;
		else
			eventData.polarity = 0;
		m_pLastADC[index] = adc12bit;

		if (m_bEventDenoisingEnabled)
		{
			m_vecEventDataPerRow.push_back(eventData);
		}
		else
		{
			m_vecEventData.push_back(eventData);
		}
	}
}

/*
*  @function: saveOpticalFlowEvent
*  @brief   :
*  @input   : col:
*             adc12bit: 
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::saveOpticalFlowEvent(int col, int adc12bit, int adc8bit)
{
	int index = m_iCurrentRow * CELEX5_COL + col;
    uint32_t tRampNo = m_uiEOTrampNo;
	//
	int adcFPN = adc12bit - m_pFpnBufferOF[index];
	if (adcFPN < 0)
		adcFPN = 0;
	else if (adcFPN > 4095)
		adcFPN = 4095;

	int It = adcFPN >> 2;
	int Et = m_iRowTimestamp % 1024;
	if (It - Et > 200) //500
		tRampNo = m_uiEOTrampNo - 1;
	else
		tRampNo = m_uiEOTrampNo;

	uint32_t tInPixelIncreasing =  adcFPN*7550/3310 + tRampNo*7550; //(adcFPN+tRampNo*3310)*7750/3310, 3310*2.281=7750¦Ìs
	if (tInPixelIncreasing > m_uiMaxInPixelTimestamp)
		m_uiMaxInPixelTimestamp = tInPixelIncreasing;
	if (tInPixelIncreasing < m_uiMinInPixelTimestamp)
		m_uiMinInPixelTimestamp = tInPixelIncreasing;

	if (!m_bEventDenoisingEnabled)
	{
		/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
		m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
		m_pEventCountBuffer[index] += 1;
		m_pEventADCBuffer[index] = tInPixelIncreasing;
		m_uiPixelCount++;
		m_uiPixelCountForEPS++;
	}
	if (m_bEventStreamEnabled)
	{
		EventData eventData;
		eventData.row = m_iCurrentRow;
		eventData.col = col;
		eventData.polarity = 0;
		eventData.adc = adcFPN;
		eventData.tOffPixelIncreasing = m_uiEventTCounterTotal * m_uiEventTUnitN / m_uiCurrentEventTUnitD;
		eventData.tInPixelIncreasing = tInPixelIncreasing;

		if (m_bEventDenoisingEnabled)
		{
			m_vecEventDataPerRow.push_back(eventData);
		}
		else
		{
			m_vecEventData.push_back(eventData);
		}
	}
	if (m_bStartGenerateOFFPN)
	{
		m_pOFFPNEventLatestValue[index] = adc12bit;
	}
}

/*
*  @function: saveFormat2Event
*  @brief   : save format2 event into buffer and event vector
*  @input   : column: column
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::saveFormat2Event(int column, int adc)
{
	if (!m_bLoopModeEnabled || (m_bLoopModeEnabled && m_uiEventTCounter >= m_uiEventTCountForRemove))
	{
		if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
		{
			if (!m_bEventDenoisingEnabled)
			{
				int index = m_iCurrentRow * CELEX5_COL + column;
				if (index < CELEX5_PIXELS_NUMBER && index >= 0)
				{
					/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
					m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
					m_pEventCountBuffer[index] += 1;
				}
				m_uiPixelCount++;
				m_uiPixelCountForEPS++;
			}
			if (m_bEventStreamEnabled)
			{
				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = column;
				eventData.adc = adc;
				//eventData.t_off_pixel = m_uiEventTCounter;
				eventData.tOffPixelIncreasing = m_uiEventTCounterTotal * m_uiEventTUnitN / m_uiCurrentEventTUnitD;

				if (m_bEventDenoisingEnabled)
				{
					m_vecEventDataPerRow.push_back(eventData);
				}
				else
				{
					m_vecEventData.push_back(eventData);
				}
			}
		}
	}
}

/*
*  @function: getCurrentIndex
*  @brief   : 
*  @input   : initIndex: 
*  @output  :
*  @return  : 
*/
int CeleX5DataProcessor::getCurrentIndex(int initIndex)
{
	switch (m_iRotateType)
	{
	case 0:
		return CELEX5_PIXELS_NUMBER - initIndex - 1;
		break;
	case 1:
		return (initIndex / CELEX5_COL * CELEX5_COL + (CELEX5_COL - initIndex % CELEX5_COL - 1));
		break;
	case 2:
		return ((CELEX5_ROW - initIndex / CELEX5_COL - 1)* CELEX5_COL + initIndex % CELEX5_COL);
		break;
	case 3:
		return initIndex;
		break;
	}
	return initIndex;
}

/*
*  @function: denoisedPerRow
*  @brief   : 
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::denoisedPerRow(bool bHasADC)
{
	int eventSize = m_vecEventDataPerRow.size();
	if (eventSize > 1)
	{
		for (int i = 0; i < eventSize; i++)
		{
			if (i == 0)
			{
				EventData eventData = m_vecEventDataPerRow[i];
				EventData eventDataR = m_vecEventDataPerRow[i + 1];
				if (abs(eventData.col - eventDataR.col) == 1)
				{
					m_vecEventData.push_back(eventData);

					int index = eventData.row * CELEX5_COL + eventData.col;
					/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
					m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
					m_pEventCountBuffer[index] += 1;
					if (bHasADC)
						m_pEventADCBuffer[index] = eventData.tInPixelIncreasing; //eventData.adc >> 4;
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;
				}
			}
			else if (i == eventSize - 1)
			{
				EventData eventDataL = m_vecEventDataPerRow[i - 1];
				EventData eventData = m_vecEventDataPerRow[i];
				if (abs(eventDataL.col - eventData.col) == 1)
				{
					m_vecEventData.push_back(eventData);

					int index = eventData.row * CELEX5_COL + eventData.col;
					/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
					m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
					m_pEventCountBuffer[index] += 1;
					if (bHasADC)
						m_pEventADCBuffer[index] = eventData.tInPixelIncreasing; //eventData.adc >> 4;
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;
				}
			}
			else
			{
				EventData eventDataL = m_vecEventDataPerRow[i - 1];
				EventData eventData = m_vecEventDataPerRow[i];
				EventData eventDataR = m_vecEventDataPerRow[i + 1];

				if (abs(eventDataL.col - eventData.col) == 1 || abs(eventData.col - eventDataR.col) == 1)
				{
					m_vecEventData.push_back(eventData);

					int index = eventData.row * CELEX5_COL + eventData.col;
					/*int value = m_pEventCountBuffer[index] + m_uiEventCountStep;
					m_pEventCountBuffer[index] = value > 255 ? 255 : value;*/
					m_pEventCountBuffer[index] += 1;
					if (bHasADC)
						m_pEventADCBuffer[index] = eventData.tInPixelIncreasing; //eventData.adc >> 4;
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;
				}
			}
		}
	}
	m_vecEventDataPerRow.clear();
}

/*
*  @function: calEventCountSlice
*  @brief   :
*  @input   :
*  @output  :
*  @return  :
*/
void CeleX5DataProcessor::calEventCountSlice(uint8_t* pEventCountSliceBuffer)
{
	int index = 0;
	int value = 0;
	if (m_uiEventCountSliceNum == 5)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);
			value = m_pEventCountBuffer[i] * 10 +
				m_pEventCountSlice[0][i] * 9 + m_pEventCountSlice[1][i] * 8 +
				m_pEventCountSlice[2][i] * 7 + m_pEventCountSlice[3][i] * 6;

			if (value > 255)
				value = 255;
			pEventCountSliceBuffer[index] = value;
		}
	}
	else if (m_uiEventCountSliceNum == 6)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			value = m_pEventCountBuffer[i] * 10 +
				m_pEventCountSlice[0][i] * 9 + m_pEventCountSlice[1][i] * 8 +
				m_pEventCountSlice[2][i] * 7 + m_pEventCountSlice[3][i] * 6 +
				m_pEventCountSlice[4][i] * 5;

			if (value > 255)
				value = 255;
			pEventCountSliceBuffer[index] = value;
		}
	}
	else if (m_uiEventCountSliceNum == 7)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			value = m_pEventCountBuffer[i] * 10 +
				m_pEventCountSlice[0][i] * 9 + m_pEventCountSlice[1][i] * 8 +
				m_pEventCountSlice[2][i] * 7 + m_pEventCountSlice[3][i] * 6 +
				m_pEventCountSlice[4][i] * 5 + m_pEventCountSlice[5][i] * 4;

			if (value > 255)
				value = 255;
			pEventCountSliceBuffer[index] = value;
		}
	}
	else if (m_uiEventCountSliceNum == 8)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			value = m_pEventCountBuffer[i] * 10 +
				m_pEventCountSlice[0][i] * 9 + m_pEventCountSlice[1][i] * 8 +
				m_pEventCountSlice[2][i] * 7 + m_pEventCountSlice[3][i] * 6 +
				m_pEventCountSlice[4][i] * 5 + m_pEventCountSlice[5][i] * 4 +
				m_pEventCountSlice[6][i] * 3;

			if (value > 255)
				value = 255;
			pEventCountSliceBuffer[index] = value;
		}
	}
}
