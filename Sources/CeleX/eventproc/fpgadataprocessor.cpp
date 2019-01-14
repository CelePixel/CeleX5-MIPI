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

#include "fpgadataprocessor.h"
#include "fpgadatareader.h"
#include "../include/celex4/celex4processeddata.h"
#include "../include/celex4/celex4datamanager.h"
#include "../base/xbase.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>
#ifndef _WIN32
#include <cstring>
#endif

#define PI (atan(1.0) * 4.0)

cv::VideoWriter			g_cvVideoWriterFullPic;
cv::VideoWriter         g_cvVideoWriterEvent;
bool					isCreateImage;
bool					isReadBin;
FrameData               g_frameData;
FrameData               g_fixedVecData;
std::vector<EventData>  g_eventData;
uint64_t                g_ulEventFrameNo = 0;
uint64_t                g_ulFixedEventNo = 0;
bool			        isGetFixedVec = false;
FPGADataProcessor::FPGADataProcessor()
	: m_bIsGeneratingFPN(false)
	, m_bAdjustBrightness(false)
	, m_iFpnCalculationTimes(0)
	, m_uiFullPicPixelCount(0)
	, m_uiOpticalFlowPixelCount(0)
	, m_uiEventPixelCount(0)
	, m_uiUpperADC(511)
	, m_uiLowerADC(0)
	, m_uiTimeSlice(750000)
	, m_uiTimeSliceValue(60)
	, m_uiClockRate(25)
	, m_uiFPGATimeCycle(131072)
	, m_uiFrameStartT(0)
	, m_uiFrameEndT(0)
	, m_uiFEEventTCounter(0)
	, m_uiFEFrameTime(0)
	, m_uiOverlapTime(0)
	, m_uiMeanIntensity(0)
	, m_ulOverlapEventTCounter(0)
	, m_ulSpecialEventCount(0)
	, m_ulSpecialByteSize(0)
	, m_emSensorMode(EventMode)
	, m_pEventPicBuffer(NULL)
	, m_pLastEventPicBuffer(NULL)
	, m_bMultiSliceEnabled(false)
	, m_bNeedResetMultiSliceData(true)
	, m_uiMultiSliceTime(1250000)
	, m_uiMultiSliceCount(255)
	, m_ulEventTCounter(0)
	, m_ulMultiSliceTCounter(0)
	, m_ulEventCountTotal(0)
	, m_uiTimeStamp(750000)
	, m_uiTimeScale(1)
    , m_emPlaybackState(NoBinPlaying)
	, m_ulVecSize(0)
	, m_ulOverlapLen(0)
{
	isCreateImage = false;
	isReadBin = false;
	m_uiEventCountStepSize = 9;
	m_pFullPicBuffer = new unsigned char[PIXELS_NUMBER];
	m_pEventPicBuffer = new unsigned char[PIXELS_NUMBER];

	m_pFpnGenerationBuffer = new long[PIXELS_NUMBER];

	m_pBufferMotion = new unsigned char[PIXELS_NUMBER];
	m_pBufferOpticalFlow = new unsigned char[PIXELS_NUMBER];
	//
	m_pMultiSliceBuffer = new long[PIXELS_NUMBER];
	m_pMultiSliceGrayBuffer = new unsigned char[PIXELS_NUMBER];
	//
	m_pFpnBuffer = new int[PIXELS_NUMBER];

	m_pOverlapEventBuffer = new long[PIXELS_NUMBER];

	m_pLastADC = new uint16_t[PIXELS_NUMBER];

	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		m_pFullPicBuffer[i] = 0;
		m_pEventPicBuffer[i] = 0;
		m_pFpnBuffer[i] = 0;
		m_pBufferMotion[i] = 0;
		m_pBufferOpticalFlow[i] = 0;
		m_pMultiSliceBuffer[i] = 0;
		m_pOverlapEventBuffer[i] = 0;

		m_pLastADC[i] = 0;
	}
	m_pCX4ProcessedData = new CeleX4ProcessedData;
	m_pCX4Server = new CX4SensorDataServer;
	m_pCX4Server->setCX4SensorData(m_pCX4ProcessedData);
}

FPGADataProcessor::~FPGADataProcessor()
{
	if (m_pFullPicBuffer)
		delete[] m_pFullPicBuffer;
	if (m_pEventPicBuffer)
		delete[] m_pEventPicBuffer;
	if (m_pFpnBuffer)
		delete[] m_pFpnBuffer;
	if (m_pFpnGenerationBuffer)
		delete[] m_pFpnGenerationBuffer;
	if (m_pMultiSliceBuffer)
		delete[] m_pMultiSliceBuffer;
	if (m_pMultiSliceGrayBuffer)
		delete[] m_pMultiSliceGrayBuffer;
}

bool FPGADataProcessor::setFpnFile(const std::string &fpnFile)
{
	int index = 0;
	std::ifstream in;
	in.open(fpnFile.c_str());
	if (!in.is_open())
	{
		return false;
	}
	std::string line;
	while (!in.eof() && index < PIXELS_NUMBER)
	{
		in >> line;
		m_pFpnBuffer[index] = atoi(line.c_str());
		index++;
	}
	if (index != PIXELS_NUMBER)
		return false;

	return true;
}

void FPGADataProcessor::processData(unsigned char *data, long length)
{
	if (!data)
	{
		return;
	}
	//--------------- Align data ---------------
	long i = 0;
	/*while (i + 1 < length)
	{
		if ((data[i] & 0x80) > 0 && (data[i+1] & 0x80) == 0x00)
		{
			i = i - 3;
			// init sRow and sT
			processEvent(&data[i]);
			break; // aligned
		}
		++i;
	}*/
	//

	//for (i = i + EVENT_SIZE; i + 11 < length; i = i + EVENT_SIZE)
	for (i = 0; i + EVENT_SIZE <= length; i = i + EVENT_SIZE)
	{
		if (FPGADataReader::isIMUData(&data[i]) == FPGADataReader::ACC_OFST_B_DATA)
		{
			//cout << "---------- IMU Data ----------" << endl;
			IMUData data = FPGADataReader::getIMUData();
			m_vectorIMUData.push_back(data);
			m_vectorIMUData_cb.push_back(data);
			if (m_vectorIMUData.size() > IMU_DATA_MAX_SIZE)
			{
				auto itr = m_vectorIMUData.begin();
				m_vectorIMUData.erase(itr);
			}
			//cout << "x = " << data.x_GYROS << ", y = " << data.y_GYROS << ", z = " << data.z_GYROS << ", t = " << data.t_GYROS << ", frameNo = " << data.frameNo << endl;
			//cout << "x = " << data.x_ACC << ", y = " << data.y_ACC << ", z = " << data.z_ACC << ", t = " << data.t_ACC << endl;
			//cout << "x = " << data.x_GYROS_OFST << ", y = " << data.y_GYROS_OFST << ", z = " << data.z_GYROS_OFST << ", t = " << data.t_GYROS_OFST << endl;
			//cout << "x = " << data.x_ACC_OFST << ", y = " << data.y_ACC_OFST << ", z = " << data.z_ACC_OFST << ", t = " << data.t_ACC_OFST << endl;
		}
		bool isSpecialEvent = !(processEvent(&data[i]));

		//cout << "------------------11" << m_ulTimeStampCount << endl;
		if (m_ulTimeStampCount > m_uiTimeStamp/* && m_emSensorMode == EventMode*/)
		{
			m_pCX4ProcessedData->setEventDataVector(m_vectorEventData);
			m_pCX4ProcessedData->setIMUDataVector(m_vectorIMUData_cb);
			g_frameData.frameNo = g_ulEventFrameNo;
			g_frameData.vecEventData.swap(m_vectorEventData);
			m_pCX4ProcessedData->setFrameDataVector(g_frameData);
			vector<EventData> tempVec;
			vector<IMUData> imuVec;
			m_vectorIMUData_cb.swap(imuVec);
			m_vectorEventData.swap(tempVec);
			m_ulTimeStampCount = 0;
		}
		if (m_fixedNumVecEventData.size() == m_ulVecSize && isGetFixedVec)
		{
			g_fixedVecData.frameNo = g_ulFixedEventNo++;
			//cout << "frame: " << g_ulFixedEventNo << endl;
			g_fixedVecData.vecEventData.swap(m_fixedNumVecEventData);
			m_pCX4ProcessedData->setFixedNumEventDataVector(g_fixedVecData);
			m_pCX4ProcessedData->setIMUDataVector(m_vectorIMUData_cb);
			vector<IMUData> imuVec;
			m_vectorIMUData_cb.swap(imuVec);
			vector<EventData> overlap;
			if (m_ulOverlapLen > 0 && m_ulOverlapLen < m_ulVecSize)
			{
				overlap.insert(overlap.begin(), m_fixedNumVecEventData.begin() + (m_ulVecSize - m_ulOverlapLen), m_fixedNumVecEventData.begin() + m_ulVecSize);
			}
			//cout << "m_vectorEventData: " << m_vectorEventData.size() << endl;
			vector<EventData> tempVec;
			m_fixedNumVecEventData.swap(overlap);
			m_pCX4Server->notify(CeleX4DataManager::CeleX_Frame_Data);
		}
		if (isSpecialEvent)
		{
			if (m_bAdjustBrightness)
			{
				adjustBrightnessImpl();
			}
			else if (m_bIsGeneratingFPN)
			{
				generateFPNimpl();
			}
			else
			{
				createImage();
				isCreateImage = true;
				if(!isGetFixedVec)
					m_pCX4Server->notify(CeleX4DataManager::CeleX_Frame_Data);
			}
			if (!isReadBin)
				cleanEventBuffer();
			if (m_emSensorMode == FullPic_Event_Mode || m_emSensorMode == FullPictureMode)
			{
				g_ulEventFrameNo++;
				if (m_emSensorMode == FullPictureMode)
				{
					m_pCX4ProcessedData->setIMUDataVector(m_vectorIMUData_cb);
					vector<IMUData> imuVec;
					m_vectorIMUData_cb.swap(imuVec);
				}
			}
		}
	}
}

bool FPGADataProcessor::processEvent(unsigned char *data)
{
	m_ulSpecialByteSize++;
	if (FPGADataReader::isSpecialEvent(data))
	{
		++m_ulSpecialEventCount;
		m_vecDataLength.push_back(m_ulSpecialByteSize * 4);
		m_ulSpecialByteSize = 0;
		if (m_ulSpecialEventCount >= 0xFFFFFFFF)
		{
			m_ulSpecialEventCount = 0;
			m_vecDataLength.clear();
			cout << "Special event counter is full!" << endl;
		}
		if (FullPictureMode == m_emSensorMode)
		{
			//cout << "FPixelCount = " << m_uiFullPicPixelCount << endl;
			m_uiFullPicPixelCount = 0;
			return false;
		}
		else if (EventMode == m_emSensorMode)
		{
			m_uiEventPixelCount = 0;
			/*
			m_vecEventCountPerSpecial.push_back(m_ulEventCountPerSpecial);
			unsigned long countPerSecond = m_ulEventCountTotal / m_ulSpecialEventCount;
			cout << "SpecialEventNo. = " << m_ulSpecialEventCount << ", EventCountPerSpecial = " << m_ulEventCountPerSpecial
				 << ", EventCountPerSecond = " << countPerSecond*100 << endl;
			*/
			m_ulEventCountPerSpecial = 0;
		}
		else if (m_emSensorMode == FullPic_Event_Mode)
		{
			//cout << "FPixelCount = " << m_uiFullPicPixelCount << ", EPixelCount = " << m_uiEventPixelCount << endl;
			m_uiFullPicPixelCount = 0;
			m_uiEventPixelCount = 0;
			m_uiOpticalFlowPixelCount = 0;
			//cout << "m_ulMultiSliceTCounter = " << m_ulMultiSliceTCounter << endl;
			return false;
		}
	}
	else if (FPGADataReader::isRowEvent(data))
	{
		return processRowEvent(data);
	}
	else if (FPGADataReader::isColumnEvent(data))
	{
		processColEvent(data);
	}
	return true;
}

bool FPGADataProcessor::processRowEvent(unsigned char* data)
{
	FPGADataReader::MapTime(data);
	if (m_emSensorMode == EventMode)
	{
		unsigned int t = FPGADataReader::getTFromFPGA();
		unsigned int tLast = FPGADataReader::getLastTFromFPGA();
		int diffT = t - tLast;
		if (diffT < 0)
		{
			//cout << "t = " << t << ", tLast = " << tLast << endl;
			diffT = diffT + m_uiFPGATimeCycle;
		}
		if (m_bMultiSliceEnabled)
			m_ulMultiSliceTCounter += diffT;

		if (m_uiOverlapTime > 0)
			m_ulOverlapEventTCounter += diffT;

		m_ulEventTCounter += diffT;
		m_ulTimeStampCount += diffT;
		//cout << "t = " << t << ", tLast = " << tLast << ", diffT = " << diffT << endl;
		FPGADataReader::setEventCounter(m_ulEventTCounter);
		if (m_ulEventTCounter > m_uiTimeSlice /*|| m_ulTimeStampCount > m_uiTimeSlice*/)
		{
			//cout << "--------------------------------------" << m_ulEventTCounter << "  " << m_uiTimeSlice << endl;
			g_ulEventFrameNo++;
			m_uiEventPixelCount = 0;
			return false;
		}
	}
	else if (m_emSensorMode == FullPic_Event_Mode)
	{
		unsigned int t = FPGADataReader::getTFromFPGA();
		unsigned int tLast = FPGADataReader::getLastTFromFPGA();
		int diffT = t - tLast;
		if (diffT < 0)
		{
			//cout << "t = " << t << ", tLast = " << tLast << endl;
			diffT = diffT + m_uiFPGATimeCycle;
		}
		if (m_uiFEEventTCounter == 0 && diffT > 10000)
		{
			//cout << "t = " << t << ", tLast = " << tLast << ", diffT = " << diffT << endl;
			diffT = 0;
		}
		m_uiFEEventTCounter += diffT;
		m_ulTimeStampCount += diffT;

		if (m_bMultiSliceEnabled)
		{
			if (0 == FPGADataReader::getEventType()) //Motion
			{
				m_ulMultiSliceTCounter += diffT;
			}
		}
	}
	return true;
}

void FPGADataProcessor::processColEvent(unsigned char* data)
{
	unsigned int row = FPGADataReader::getCurrentRow();
	unsigned int col = FPGADataReader::getColumn(data);
	unsigned int adc = FPGADataReader::getBrightness(data);
	unsigned int t = FPGADataReader::getTFromFPGA();
	EventData eData;

	if (row < PIXELS_PER_ROW && col < PIXELS_PER_COL)
	{
		// normalize to 0~255
		unsigned int adc1 = normalizeADC(adc);
		int index = row * PIXELS_PER_COL + col;

		eData.row = row;
		eData.col = col;
		eData.brightness = adc1;

		if (m_emSensorMode == FullPictureMode)
		{
			m_pFullPicBuffer[index] = adc1;
			//eData.t = t;
			m_uiFullPicPixelCount++;
		}
		else if (m_emSensorMode == EventMode)
		{
			if (adc > m_pLastADC[index])
				eData.polarity = 1;
			else if (adc < m_pLastADC[index])
				eData.polarity = -1;
			else
				eData.polarity = 0;
			m_pLastADC[index] = adc;

			m_pFullPicBuffer[index] = adc1;
			m_pEventPicBuffer[index] += m_uiEventCountStepSize;
			eData.t = m_ulTimeStampCount / m_uiTimeScale;

			if (m_bMultiSliceEnabled)
			{
				m_pMultiSliceBuffer[index] = m_ulMultiSliceTCounter;
			}
			if (m_uiOverlapTime > 0)
			{
				m_pOverlapEventBuffer[index] = m_ulOverlapEventTCounter;
			}
			m_uiEventPixelCount++;
			m_ulEventCountPerSpecial++;
			m_ulEventCountTotal++;
			//
			m_vectorEventData.push_back(eData);
			m_fixedNumVecEventData.push_back(eData);
		}
		else if (m_emSensorMode == FullPic_Event_Mode)
		{
			unsigned int type = FPGADataReader::getEventType();
			if (0 == type) //Motion
			{
				if (m_uiFrameEndT == 0 ||
					(m_uiFEEventTCounter >= m_uiFrameStartT && m_uiFEEventTCounter < m_uiFrameEndT))
				{
					m_pBufferMotion[index] += m_uiEventCountStepSize;
					m_pMultiSliceBuffer[index] = m_ulMultiSliceTCounter;
				}
				m_uiEventPixelCount++;
				eData.t = m_ulTimeStampCount / m_uiTimeScale;
			}
			else if (1 == type) //Full Picture
			{
				m_pFullPicBuffer[index] = adc1;
				m_uiFullPicPixelCount++;
			}
			else if (2 == type) //Optical Flow
			{
				m_pBufferOpticalFlow[index] = adc1; //for optical-flow A is T
				m_uiOpticalFlowPixelCount++;
			}
			m_vectorEventData.push_back(eData);
		}
	}
}

void FPGADataProcessor::createImage()
{
	if (FullPictureMode == m_emSensorMode)
	{
		unsigned long sumIntensity = 0;
		unsigned char* pDesFullPic = m_pCX4ProcessedData->getFullPicBuffer();
		if (NULL == pDesFullPic)
			return;
		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			//--- full picture buffer ---
			int value = m_pFullPicBuffer[i] - m_pFpnBuffer[i]; //Subtract FPN
			if (value < 0)
				value = 0;
			if (value > 255)
				value = 255;
			pDesFullPic[PIXELS_NUMBER - i - 1] = value;
			sumIntensity += value;
		}
		m_uiMeanIntensity = sumIntensity / PIXELS_NUMBER;
		if (g_cvVideoWriterFullPic.isOpened())
		{
			cv::Mat mat(640, 768, CV_8UC1, pDesFullPic);
			g_cvVideoWriterFullPic.write(mat);
		}
	}
	else if (EventMode == m_emSensorMode)
	{
		int multiSliceTDiff = 0;
		unsigned int uiTPerSlice = 0;
		if (m_emSensorMode == EventMode)
		{
			if (m_uiOverlapTime > 0)
			{
				if (m_ulOverlapEventTCounter > m_uiOverlapTime + m_uiTimeSlice)
				{
					m_ulOverlapEventTCounter -= m_uiTimeSlice;
				}
			}
			if (m_bMultiSliceEnabled)
			{
				//cout << "--------" << m_ulMultiSliceTCounter << "  " << m_uiMultiSliceTime << "  " << m_ulEventTCounter << endl;
				if (m_ulMultiSliceTCounter > m_uiMultiSliceTime)
				{
					if (m_bNeedResetMultiSliceData)
					{
						multiSliceTDiff = m_ulMultiSliceTCounter - m_uiMultiSliceTime + m_uiTimeSlice;
						m_ulMultiSliceTCounter = m_uiMultiSliceTime - m_uiTimeSlice;
						m_bNeedResetMultiSliceData = false;
					}
					else
					{
						m_ulMultiSliceTCounter -= m_ulEventTCounter;
						multiSliceTDiff = m_ulEventTCounter;
					}
				}
				uiTPerSlice = m_uiMultiSliceTime / m_uiMultiSliceCount;
			}
			m_ulEventTCounter = 0;
		}
		unsigned long sumIntensity = 0;
		unsigned char* pDesEAccumulatedPic = m_pCX4ProcessedData->getEventPicBuffer(EventAccumulatedPic);
		unsigned char* pDesEBinarydPic = m_pCX4ProcessedData->getEventPicBuffer(EventBinaryPic);
		unsigned char* pDesEGrayPic = m_pCX4ProcessedData->getEventPicBuffer(EventGrayPic);
		unsigned char* pDesESuperimposedPic = m_pCX4ProcessedData->getEventPicBuffer(EventSuperimposedPic);
		unsigned char* pDesEDenoisedBinaryPic = m_pCX4ProcessedData->getEventPicBuffer(EventDenoisedBinaryPic);
		unsigned char* pDesEDenoisedGrayPic = m_pCX4ProcessedData->getEventPicBuffer(EventDenoisedGrayPic);
		unsigned char* pDesECountPic = m_pCX4ProcessedData->getEventPicBuffer(EventCountPic);
		//unsigned char* pDesEDenoisedByTimeBinaryPic = m_pCX4ProcessedData->getEventPicBuffer(EventDenoisedByTimeBinaryPic);
		//unsigned char* pDesEDenoisedByTimeGrayPic = m_pCX4ProcessedData->getEventPicBuffer(EventDenoisedByTimeGrayPic);
		unsigned char* pDesOpticalRawPic = m_pCX4ProcessedData->getOpticalFlowPicBuffer();

		//binary and gray after denoising
		//denoisingByTimeInterval(g_frameData.vecEventData, pDesEDenoisedByTimeBinaryPic, EventBinaryPic);
		//denoisingByTimeInterval(g_frameData.vecEventData, pDesEDenoisedByTimeGrayPic, EventGrayPic);

		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			//--- full picture buffer ---
			int value = m_pFullPicBuffer[i] - m_pFpnBuffer[i]; //Subtract FPN
			if (value < 0)
				value = 0;
			if (value > 255)
				value = 255;
			pDesEAccumulatedPic[PIXELS_NUMBER - i - 1] = value;
			//--- event picture buffer ---
			if (m_uiOverlapTime > 0)
			{
				if (m_pOverlapEventBuffer[i] > 0)
				{
					m_pOverlapEventBuffer[i] = m_pOverlapEventBuffer[i] - m_uiTimeSlice;
					if (m_pOverlapEventBuffer[i] < 0)
						m_pOverlapEventBuffer[i] = 0;
				}
				else
				{
					m_pOverlapEventBuffer[i] = 0;
				}
				pDesEBinarydPic[PIXELS_NUMBER - i - 1] = m_pOverlapEventBuffer[i] > 0 ? 255 : 0; //binary pic
				pDesEGrayPic[PIXELS_NUMBER - i - 1] = m_pOverlapEventBuffer[i] > 0 ? value : 0; //gray pic
			}
			else
			{
				pDesEBinarydPic[PIXELS_NUMBER - i - 1] = m_pEventPicBuffer[i] > 0 ? 255 : 0; //binary pic
				pDesEGrayPic[PIXELS_NUMBER - i - 1] = m_pEventPicBuffer[i] > 0 ? value : 0; //gray pic
			}
			pDesESuperimposedPic[PIXELS_NUMBER - i - 1] = pDesEDenoisedBinaryPic[PIXELS_NUMBER - i - 1] > 0 ? 255 : value; //superimposed pic
			pDesECountPic[PIXELS_NUMBER - i - 1] = m_pEventPicBuffer[i];	//event count pic
			//--- denoised pic ---
			int score = m_pEventPicBuffer[i];
			if (score > 0)
				score = calculateDenoiseScore(m_pEventPicBuffer, m_pLastEventPicBuffer, i);
			else
				score = 0;

			pDesEDenoisedBinaryPic[PIXELS_NUMBER - i - 1] = score; //denoised binary pic
			pDesEDenoisedGrayPic[PIXELS_NUMBER - i - 1] = score > 0 ? value : 0; //denoised gray pic

			if (m_bMultiSliceEnabled)
			{
				if (m_pMultiSliceBuffer[i] > 0 && uiTPerSlice > 0)
				{
					int slice = m_pMultiSliceBuffer[i] / uiTPerSlice + 1;
					m_pMultiSliceGrayBuffer[PIXELS_NUMBER - i - 1] = slice;
					//
					m_pMultiSliceBuffer[i] = m_pMultiSliceBuffer[i] - multiSliceTDiff;
					if (m_pMultiSliceBuffer[i] < 0)
						m_pMultiSliceBuffer[i] = 0;
				}
				else
				{
					m_pMultiSliceGrayBuffer[PIXELS_NUMBER - i - 1] = 0;
					m_pMultiSliceBuffer[i] = 0;
				}
			}
			sumIntensity += value;
		}
		m_uiMeanIntensity = sumIntensity / PIXELS_NUMBER;
		if (NULL == m_pLastEventPicBuffer)
		{
			m_pLastEventPicBuffer = new unsigned char[PIXELS_NUMBER];
		}
		memcpy(m_pLastEventPicBuffer, m_pEventPicBuffer, PIXELS_NUMBER);
		if (m_bMultiSliceEnabled)
		{
			//--- denoised pic ---
			for (int i = 0; i < PIXELS_NUMBER; ++i)
			{
				if (m_pMultiSliceGrayBuffer[i] > 0)
				{
					if (0 == calculateDenoiseScore(m_pMultiSliceGrayBuffer, i))
						m_pMultiSliceGrayBuffer[i] = 0;
				}
			}
			calDirectionAndSpeed();
			memcpy(pDesOpticalRawPic, m_pMultiSliceGrayBuffer, PIXELS_NUMBER);
		}
		if (g_cvVideoWriterEvent.isOpened())
		{
			cv::Mat mat(640, 768, CV_8UC1, pDesEBinarydPic);
			g_cvVideoWriterEvent.write(mat);
		}
	}
	else if (FullPic_Event_Mode == m_emSensorMode)
	{
		unsigned char* pDesFullPic = m_pCX4ProcessedData->getFullPicBuffer();
		unsigned char* pDesEBinarydPic = m_pCX4ProcessedData->getEventPicBuffer(EventBinaryPic);
		unsigned char* pDesEGrayPic = m_pCX4ProcessedData->getEventPicBuffer(EventGrayPic);
		unsigned char* pDesESuperimposedPic = m_pCX4ProcessedData->getEventPicBuffer(EventSuperimposedPic);
		unsigned char* pDesEDenoisedBinaryPic = m_pCX4ProcessedData->getEventPicBuffer(EventDenoisedBinaryPic);
		unsigned char* pDesEDenoisedGrayPic = m_pCX4ProcessedData->getEventPicBuffer(EventDenoisedGrayPic);
		unsigned char* pDesECountPic = m_pCX4ProcessedData->getEventPicBuffer(EventCountPic);
		unsigned char* pDesOpticalRawPic = m_pCX4ProcessedData->getOpticalFlowPicBuffer();
		int multiSliceTDiff = 0;
		unsigned int uiTPerSlice = m_ulMultiSliceTCounter / m_uiMultiSliceCount;
		//cout << "uiTPerSlice = " << uiTPerSlice << endl;
		unsigned long sumIntensity = 0;
		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			//--- full picture buffer ---
			int value = m_pFullPicBuffer[i] - m_pFpnBuffer[i]; //Subtract FPN
			if (value < 0)
				value = 0;
			if (value > 255)
				value = 255;
			pDesFullPic[PIXELS_NUMBER - i - 1] = value;

			sumIntensity += value;

			//--- event picture buffer ---
			pDesECountPic[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i]; //event count pic
			pDesEBinarydPic[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 0 ? 255 : 0;
			pDesEGrayPic[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 0 ? value : 0;
			//pDesESuperimposedPic[PIXELS_NUMBER - i - 1] = m_pBufferMotion[i] > 0 ? 255 : value;

			//--- denoised pic ---
			int score = m_pBufferMotion[i];
			if (score > 0)
				score = calculateDenoiseScore(m_pBufferMotion, m_pLastEventPicBuffer, i);
			else
				score = 0;
			pDesEDenoisedBinaryPic[PIXELS_NUMBER - i - 1] = score; //denoised binary pic
			pDesEDenoisedGrayPic[PIXELS_NUMBER - i - 1] = score > 0 ? value : 0; //denoised gray pic
			pDesESuperimposedPic[PIXELS_NUMBER - i - 1] = pDesEDenoisedBinaryPic[PIXELS_NUMBER - i - 1] > 0 ? 255 : value;

			if (m_bMultiSliceEnabled)
			{
				if (m_pMultiSliceBuffer[i] > 0 && uiTPerSlice > 0)
				{
					int slice = m_pMultiSliceBuffer[i] / uiTPerSlice + 1;
					m_pMultiSliceGrayBuffer[PIXELS_NUMBER - i - 1] = slice;
				}
				else
				{
					m_pMultiSliceGrayBuffer[PIXELS_NUMBER - i - 1] = 0;
				}
				m_pMultiSliceBuffer[i] = 0;
			}
		}
		if (m_bMultiSliceEnabled)
		{
			m_ulMultiSliceTCounter = 0;
			//--- denoised pic ---
			for (int i = 0; i < PIXELS_NUMBER; ++i)
			{
				if (m_pMultiSliceGrayBuffer[i] > 0)
				{
					if (0 == calculateDenoiseScore(m_pMultiSliceGrayBuffer, i))
						m_pMultiSliceGrayBuffer[i] = 0;
				}
			}
			calDirectionAndSpeed();
			memcpy(pDesOpticalRawPic, m_pMultiSliceGrayBuffer, PIXELS_NUMBER);
		}
		m_uiMeanIntensity = sumIntensity / PIXELS_NUMBER;

		if (NULL == m_pLastEventPicBuffer)
		{
			m_pLastEventPicBuffer = new unsigned char[PIXELS_NUMBER];
		}
		memcpy(m_pLastEventPicBuffer, m_pBufferMotion, PIXELS_NUMBER);

		if (g_cvVideoWriterFullPic.isOpened())
		{
			cv::Mat mat(640, 768, CV_8UC1, pDesFullPic);
			g_cvVideoWriterFullPic.write(mat);
		}
		if (g_cvVideoWriterEvent.isOpened())
		{
			cv::Mat mat(640, 768, CV_8UC1, pDesEBinarydPic);
			g_cvVideoWriterEvent.write(mat);
		}
		m_uiFEEventTCounter = 0;
	}
}

void FPGADataProcessor::cleanEventBuffer()
{
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		m_pEventPicBuffer[i] = 0;
		m_pBufferMotion[i] = 0;
	}
}

void FPGADataProcessor::setSensorMode(emSensorMode mode)
{
	m_emSensorMode = mode;
	m_ulMultiSliceTCounter = 0;
	m_bNeedResetMultiSliceData = true;
	m_pCX4ProcessedData->setSensorMode(mode);
}

emSensorMode FPGADataProcessor::getSensorMode()
{
	return m_emSensorMode;
}

void FPGADataProcessor::generateFPN(std::string filePath)
{
	m_bIsGeneratingFPN = true;
	m_iFpnCalculationTimes = FPN_CALCULATION_TIMES;
	m_strFpnFilePath = filePath;
}

void FPGADataProcessor::adjustBrightness()
{
	m_bAdjustBrightness = (!m_bAdjustBrightness);
}

void FPGADataProcessor::setUpperADC(uint32_t value)
{
	m_uiUpperADC = value;
}
uint32_t FPGADataProcessor::getUpperADC()
{
	return m_uiUpperADC;
}

void FPGADataProcessor::setLowerADC(uint32_t value)
{
	m_uiLowerADC = value;
}
uint32_t FPGADataProcessor::getLowerADC()
{
	return m_uiLowerADC;
}

void FPGADataProcessor::setTimeSlice(uint32_t msec)
{
	m_uiTimeSliceValue = msec;
	m_uiTimeStamp = m_uiTimeSlice = m_uiClockRate * 1000 * msec / 2;
	m_ulMultiSliceTCounter = 0;
	m_bNeedResetMultiSliceData = true;
}

void FPGADataProcessor::setOverlapTime(uint32_t msec)
{
	m_uiOverlapTime = m_uiClockRate * 1000 * msec / 2; //25000 * msec;
}

void FPGADataProcessor::setFrameLengthRange(float startRatio, float endRatio)
{
	uint32_t emsec = 0;
	if (m_uiFEFrameTime > 20)
		emsec = m_uiFEFrameTime - 20;
	m_uiFrameStartT = startRatio * m_uiClockRate * 1000 * emsec / 2; //25000 * msec
	m_uiFrameEndT = endRatio * m_uiClockRate * 1000 * emsec / 2; //25000 * msec
}

void FPGADataProcessor::setFEFrameTime(uint32_t msec)
{
	m_uiFEFrameTime = msec;
}

CeleX4ProcessedData *FPGADataProcessor::getSensorDataObject()
{
	return m_pCX4ProcessedData;
}

CX4SensorDataServer *FPGADataProcessor::getSensorDataServer()
{
	return m_pCX4Server;
}

void FPGADataProcessor::setClockRate(uint32_t value)
{
	m_uiClockRate = value;
	m_uiTimeStamp = m_uiTimeSlice = m_uiClockRate * 1000 * m_uiTimeSliceValue / 2; //25000 * msec;
}

void FPGADataProcessor::setFPGATimeCycle(uint32_t value)
{
	m_uiFPGATimeCycle = value;
}

void FPGADataProcessor::enableMultiSlice(bool enable)
{
	m_bMultiSliceEnabled = enable;
	m_ulMultiSliceTCounter = 0;
	m_bNeedResetMultiSliceData = true;
}

bool FPGADataProcessor::isMultiSliceEnabled()
{
	return m_bMultiSliceEnabled;
}

void FPGADataProcessor::setMultiSliceTime(uint32_t msec)
{
	m_uiMultiSliceTime = m_uiClockRate * 1000 * msec / 2; //25000 * msec;
	m_ulMultiSliceTCounter = 0;
	m_bNeedResetMultiSliceData = true;
}

void FPGADataProcessor::setMultiSliceCount(uint32_t count)
{
	m_uiMultiSliceCount = count;
}

void FPGADataProcessor::generateFPNimpl()
{
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		if (m_iFpnCalculationTimes == FPN_CALCULATION_TIMES)
		{
			m_pFpnGenerationBuffer[i] = m_pFullPicBuffer[i];
		}
		else
		{
			m_pFpnGenerationBuffer[i] += m_pFullPicBuffer[i];
		}
	}
	--m_iFpnCalculationTimes;

	if (m_iFpnCalculationTimes <= 0)
	{
		m_bIsGeneratingFPN = false;
		std::ofstream ff;
		if (m_strFpnFilePath.empty())
		{
			XBase base;
			std::string filePath = base.getApplicationDirPath();
			filePath += "\\FPN.txt";
			// output the FPN file now
			ff.open(filePath.c_str());
		}
		else
		{
			ff.open(m_strFpnFilePath.c_str());
		}
		if (!ff)
			return;
		long total = 0;
		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			m_pFpnGenerationBuffer[i] = m_pFpnGenerationBuffer[i] / FPN_CALCULATION_TIMES;
			total += m_pFpnGenerationBuffer[i];
		}
		int avg = total / PIXELS_NUMBER;
		for (int i = 0; i < PIXELS_NUMBER; ++i)
		{
			int d = m_pFpnGenerationBuffer[i] - avg;
			ff << d;
			ff << "\n";
		}
		ff.close();
	}
}

void FPGADataProcessor::adjustBrightnessImpl()
{
	long total = 0;
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		total += m_pFullPicBuffer[i];
	}
	long avg = total / PIXELS_NUMBER;

	if (avg < 118)
	{
		//emit commandCallback("up_brightness");
	}
	if (avg > 138)
	{
		//emit commandCallback("down_brightness");
	}
}

// normalize to 0~255
unsigned int FPGADataProcessor::normalizeADC(unsigned int adc)
{
	int brightness = adc;
	if (adc < m_uiLowerADC)
		brightness = 0;
	else if (adc > m_uiUpperADC)
		brightness = 255;
	else
		brightness = 255 * (adc - m_uiLowerADC) / (m_uiUpperADC - m_uiLowerADC);
	return brightness;
}

void FPGADataProcessor::calDirectionAndSpeed()
{
	unsigned char* pDesOpticalDPic = m_pCX4ProcessedData->getOpticalFlowDirectionPicBuffer();
	unsigned char* pDesOpticalSPic = m_pCX4ProcessedData->getOpticalFlowSpeedPicBuffer();
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		int row = i / PIXELS_PER_COL;
		int col = i % PIXELS_PER_COL;
		int Gx = 0, Gy = 0;

		if (col == 0 || col == 767)
			Gx = 0;
		else
			Gx = m_pMultiSliceGrayBuffer[i + 1] - m_pMultiSliceGrayBuffer[i - 1];

		if (row == 0 || row == 639)
			Gy = 0;
		else
			Gy = m_pMultiSliceGrayBuffer[i + 768] - m_pMultiSliceGrayBuffer[i - 768];

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
				theta = atan2(Gy, Gx) * 180 / PI;
			}
		}
		if (theta < 0)
			theta += 360;
		pDesOpticalDPic[i] = theta * 255 / 360;

		int value1 = sqrt(Gx*Gx + Gy*Gy);
		if (value1 > 255)
			value1 = 255;
		pDesOpticalSPic[i] = value1;
	}
}

int FPGADataProcessor::calculateDenoiseScore(unsigned char* pBuffer, unsigned char* pBufferPre, unsigned int pos)
{
	if (NULL == pBuffer || NULL == pBufferPre)
	{
		return 255;
	}
	int row = pos / PIXELS_PER_COL;
	int col = pos % PIXELS_PER_COL;

	int count1 = 0;
	int count2 = 0;
	int count3 = 0;
	int count4 = 0;
	for (int i = row - 1; i < row + 2; ++i) //8 points
	{
		for (int j = col - 1; j < col + 2; ++j)
		{
			int index = i * PIXELS_PER_COL + j;
			if (index < 0 || index == pos || index >= PIXELS_NUMBER)
				continue;
			if (pBuffer[index] > 0)
				++count1;
			else
				++count2;
		}
	}
	int index1 = (row - 1)*PIXELS_PER_COL + col;
	int index2 = row*PIXELS_PER_COL + col - 1;
	int index3 = row*PIXELS_PER_COL + col + 1;
	int index4 = (row + 1)*PIXELS_PER_COL + col;
	int aa[4] = { index1, index2, index3, index4 };
	for (int i = 0; i < 4; ++i)
	{
		if (aa[i] < 0 || aa[i] >= PIXELS_NUMBER)
			continue;
		if (pBufferPre[aa[i]] > 0)
			++count3;
		else
			++count4;
	}
	if (count1 >= count2 || count3 >= count4)
		return 255;
	else
		return 0;
}

void FPGADataProcessor::setPlaybackState(PlaybackState state)
{	
	m_emPlaybackState = state;
	if (state == Playing)
	{
		m_ulSpecialByteSize = 0;
		m_ulSpecialEventCount = 0;
		m_vecDataLength.clear();
	}
}

int FPGADataProcessor::calculateDenoiseScore(unsigned char* pBuffer, unsigned int pos)
{
	if (NULL == pBuffer)
	{
		return 255;
	}
	int row = pos / PIXELS_PER_COL;
	int col = pos % PIXELS_PER_COL;

	int count1 = 0;
	int count2 = 0;
	for (int i = row - 1; i < row + 2; ++i) //8 points
	{
		for (int j = col - 1; j < col + 2; ++j)
		{
			int index = i * PIXELS_PER_COL + j;
			if (index < 0 || index == pos || index >= PIXELS_NUMBER)
				continue;
			if (pBuffer[index] > 0)
				++count1;
			else
				++count2;
		}
	}
	if (count1 >= count2)
		return 255;
	else
		return 0;
}

unsigned long FPGADataProcessor::getSpecialEventCount()
{
	return m_ulSpecialEventCount;
}

void FPGADataProcessor::setSpecialEventCount(unsigned long count)
{
	m_ulSpecialEventCount = count;
}

std::vector<int> FPGADataProcessor::getDataLengthPerSpecial()
{
	return m_vecDataLength;
}

std::vector<unsigned long> FPGADataProcessor::getEventCountListPerSpecial()
{
	return m_vecEventCountPerSpecial;
}

uint32_t FPGADataProcessor::getMeanIntensity()
{
	return m_uiMeanIntensity;
}

/*************************************************
Fuction:	getBinFileAttributes
Description:	get attributes from the bin file
Input:	binFile - the path of the bin file
Return:	BinFileAttributes - a struct of the attributes
*************************************************/
BinFileAttributes FPGADataProcessor::getBinFileAttributes(std::string binFile)
{
	char header[8];

	//open the bin file
	if (m_ifstreamBin.is_open())
	{
		m_ifstreamBin.close();
	}
	m_ifstreamBin.open(binFile.c_str(), std::ios::binary);
	if (!m_ifstreamBin.good())
	{
		cout << "Can't Open File: " << binFile.c_str();
	}

	//read the file
	m_ifstreamBin.read(header, 8);
	m_ifstreamBin.seekg(0, ios::end);
	BinFileAttributes binFileAttributes;
	binFileAttributes.second = header[0];
	binFileAttributes.minute = header[1];
	binFileAttributes.hour = header[2];
	binFileAttributes.mode = (emSensorMode)header[7];
	binFileAttributes.length = m_ifstreamBin.tellg();
	//binFileAttributes.clockRate = header[6];
	m_ifstreamBin.close();//close 
	return binFileAttributes;
}

void FPGADataProcessor::setTimeScale(float scale)
{
	//m_uiTimeScale = scale * 100000 / 8;
	m_uiTimeScale = scale * 500 * m_uiClockRate; //(scale * m_uiClockRate) / (25 * 0.08us)
}

void FPGADataProcessor::setVecSizeAndOverlap(unsigned long vecSize, unsigned long overlap)
{
	isGetFixedVec = true;
	m_ulOverlapLen = overlap;
	m_ulVecSize = vecSize;
	//cout << "size:" << m_ulVecSize << ", overlap:" << overlap << endl;
}

/*************************************************
Fuction:	setEventCountStepSize
Description:	set the step size for event count
Input:	stepSize - step size(default:9)
Return:	void
*************************************************/
void FPGADataProcessor::setEventCountStepSize(uint32_t stepSize)
{
	m_uiEventCountStepSize = stepSize;
}

int FPGADataProcessor::getIMUDataSize()
{
	return m_vectorIMUData.size();
}

int FPGADataProcessor::getIMUData(int count, std::vector<IMUData>& data)
{
	if (m_vectorIMUData.empty() || count <= 0)
	{
		return 0;
	}
	else
	{
		if (count >= m_vectorIMUData.size())
		{
			m_vectorIMUData.swap(data);
			vector<IMUData> tempVec;
			m_vectorIMUData.swap(tempVec);
			return data.size();
		}
		else
		{
			int index = 0;
			for (auto itr = m_vectorIMUData.begin(); itr != m_vectorIMUData.end();)
			{
				index++;
				data.push_back(m_vectorIMUData.front());
				itr = m_vectorIMUData.erase(itr);
				if (index == count)
					break;
			}
			return count;
		}
	}
}

int FPGADataProcessor::getIMUData(std::vector<IMUData>& data)
{
	m_vectorIMUData.swap(data);
	vector<IMUData> tempVec;
	m_vectorIMUData.swap(tempVec);
	return data.size();
}

void FPGADataProcessor::denoisingByTimeInterval(std::vector<EventData> vec, unsigned char* buffer, emEventPicMode mode)
{
	cv::Mat preImg = cv::Mat::zeros(640, 768, CV_32FC1);
	//!!!clear
	for (int i = 0; i < PIXELS_NUMBER; ++i)
	{
		buffer[i] = 0;
	}

	if (!vec.empty())
	{
		for (int t = 0; t < vec.size() - 1; ++t)
		{
			if (vec[t].row == 0 || vec[t].row == 639 || vec[t].col == 0 || vec[t].col == 767)
				continue;

			float d1 = preImg.at<float>(vec[t].row, vec[t].col);
			float d2 = preImg.at<float>(vec[t].row, vec[t].col + 1);
			float d3 = preImg.at<float>(vec[t].row + 1, vec[t].col);
			float d4 = preImg.at<float>(vec[t].row, vec[t].col - 1);
			float d5 = preImg.at<float>(vec[t].row - 1, vec[t].col);

			float deltaT = 5 * (vec[t].t + preT) - d1 - d2 - d3 - d4 - d5;

			count += 1;
			tSum += deltaT;

			if (deltaT > FLT_MAX || deltaT < FLT_MIN)
			{
				count = 0.0;
				tSum = 0.0;
				preT = 0.0;
				preImg = cv::Mat::zeros(640, 768, CV_32FC1);
			}

			if (deltaT < tSum / count / 2.0)
			{
				if (mode == EventBinaryPic)
				{
					buffer[PIXELS_NUMBER - (vec[t].row * PIXELS_PER_COL + vec[t].col) - 1] = 255;
				}
				else if (mode == EventGrayPic)
				{
					buffer[PIXELS_NUMBER - (vec[t].row * PIXELS_PER_COL + vec[t].col) - 1] = vec[t].brightness;
				}
			}
			preImg.at<float>(vec[t].row, vec[t].col) = vec[t].t + preT;
		}
		preT += vec[vec.size() - 1].t;
	}
}

