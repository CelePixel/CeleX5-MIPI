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

#include "celex5dataprocessor.h"
#include "../base/xbase.h"
#include <iostream>
#include <sstream>

//#define _ENABLE_LOG_FILE_

using namespace std;

CeleX5DataProcessor::CeleX5DataProcessor()
	: m_uiPixelCount(0)
	, m_uiPixelCountForEPS(0)
	, m_uiEventNumberEPS(0)
	, m_uiEventTCounter(0)
	, m_uiEventTCounter_Total(0)
	, m_uiEventTCounter_EPS(0)
	, m_uiEventTCountForShow(1500)
	, m_uiEventTCountForRemove(0)
	, m_uiEventTCountForEPS(50000)
	, m_uiEventCountForShow(100000)
	, m_uiEventRowCycleCountForShow(6)
	, m_iLastRow(-1)
	, m_iCurrentRow(-1)
	, m_uiRowCount(0)
	, m_bIsGeneratingFPN(false)
	, m_iFpnCalculationTimes(-1)
	, m_bLoopModeEnabled(false)
	, m_uiISOLevel(2)
	, m_iLastRowTimeStamp(-1)
	, m_iRowTimeStamp(-1)
	, m_iMIPIDataFormat(2)
	, m_emSensorFixedMode(CeleX5::Event_Address_Only_Mode)
	, m_emSensorLoopAMode(CeleX5::Full_Picture_Mode)
	, m_emSensorLoopBMode(CeleX5::Event_Address_Only_Mode)
	, m_emSensorLoopCMode(CeleX5::Full_Optical_Flow_S_Mode)
	, m_uiEventFrameTime(30000)
	, m_uiEventRowCycleCount(0)
	, m_emEventShowType(EventShowByTime)
	, m_uiEventTUnitList{ 10, 25, 22, 25, 20, 17, 14, 25, 22, 20, 10, 10, 10, 10, 10, 10 }
	, m_uiCurrentEventTUnit(20)
	, m_uiEventFrameNo(0)
	, m_uiEventCountStep(30)
	, m_lEventFrameTimeStamp(0)
	, m_lLastPackageTimeStamp(0)
	, m_uiPackageTCounter(0)
	, m_bSaveFullPicRawData(false)
	, m_iRotateType(0)
	, m_bFrameModuleEnabled(true)
	, m_bIMUModuleEnabled(true)
	, m_lFullFrameTimeStamp_ForUser(0)
	, m_lEventFrameTimeStamp_ForUser(0)
	, m_lOpticalFrameTimeStamp_ForUser(0)
{
	m_pEventADCBuffer = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventCountBuffer = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pLastADC = new uint16_t[CELEX5_PIXELS_NUMBER];

	m_pFpnGenerationBuffer = new long[CELEX5_PIXELS_NUMBER];
	m_pFpnBuffer = new int[CELEX5_PIXELS_NUMBER];
	//

	m_pFullFrameBuffer_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];

	m_pEventFrameBuffer1_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventFrameBuffer2_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventFrameBuffer3_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventFrameBuffer4_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventFrameBuffer5_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventFrameBuffer6_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventFrameBuffer7_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];

	m_pOpticalFrameBuffer1_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pOpticalFrameBuffer2_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pOpticalFrameBuffer3_ForUser = new unsigned char[CELEX5_PIXELS_NUMBER];

	for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
	{
		m_pEventADCBuffer[i] = 0;
		m_pEventCountBuffer[i] = 0;
		m_pLastADC[i] = 0;
		//
		m_pFpnGenerationBuffer[i] = 0;
		m_pFpnBuffer[i] = 0;
		//
		m_pFullFrameBuffer_ForUser[i] = 0;
		m_pEventFrameBuffer1_ForUser[i] = 0;
		m_pEventFrameBuffer2_ForUser[i] = 0;
		m_pEventFrameBuffer3_ForUser[i] = 0;
		m_pEventFrameBuffer4_ForUser[i] = 0;
		m_pEventFrameBuffer5_ForUser[i] = 0;
		m_pEventFrameBuffer6_ForUser[i] = 0;
		m_pEventFrameBuffer7_ForUser[i] = 0;
		m_pOpticalFrameBuffer1_ForUser[i] = 0;
		m_pOpticalFrameBuffer2_ForUser[i] = 0;
		m_pOpticalFrameBuffer3_ForUser[i] = 0;
	}
	m_pCX5ProcessedData = new CeleX5ProcessedData;
	m_pCX5Server = new CX5SensorDataServer;
	m_pCX5Server->setCX5SensorData(m_pCX5ProcessedData);

#ifdef _ENABLE_LOG_FILE_
	if (!m_ofLogFile.is_open())
		m_ofLogFile.open("log_file_time.txt");
#endif
}

CeleX5DataProcessor::~CeleX5DataProcessor()
{
	if (m_pCX5ProcessedData) delete m_pCX5ProcessedData;
	if (m_pCX5Server) delete m_pCX5Server;

	if (m_pEventCountBuffer) delete[] m_pEventCountBuffer;
	if (m_pEventADCBuffer) delete[] m_pEventADCBuffer;
	if (m_pFpnGenerationBuffer) delete[] m_pFpnGenerationBuffer;
	if (m_pFpnBuffer) delete[] m_pFpnBuffer;
}

void CeleX5DataProcessor::getFullPicBuffer(unsigned char* buffer)
{
	memcpy(buffer, m_pFullFrameBuffer_ForUser, CELEX5_PIXELS_NUMBER);
}

void CeleX5DataProcessor::getFullPicBuffer(unsigned char* buffer, std::time_t& time_stamp)
{
	memcpy(buffer, m_pFullFrameBuffer_ForUser, CELEX5_PIXELS_NUMBER);
	time_stamp = m_lFullFrameTimeStamp_ForUser;
}

void CeleX5DataProcessor::getEventPicBuffer(unsigned char* buffer, CeleX5::emEventPicType type)
{
	if (type == CeleX5::EventBinaryPic)
		memcpy(buffer, m_pEventFrameBuffer1_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventGrayPic)
		memcpy(buffer, m_pEventFrameBuffer2_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventAccumulatedPic)
		memcpy(buffer, m_pEventFrameBuffer3_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventSuperimposedPic)
		memcpy(buffer, m_pEventFrameBuffer4_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedBinaryPic)
		memcpy(buffer, m_pEventFrameBuffer5_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountPic)
		memcpy(buffer, m_pEventFrameBuffer6_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedCountPic)
		memcpy(buffer, m_pEventFrameBuffer7_ForUser, CELEX5_PIXELS_NUMBER);
}

void CeleX5DataProcessor::getEventPicBuffer(unsigned char* buffer, std::time_t& time_stamp, CeleX5::emEventPicType type)
{
	if (type == CeleX5::EventBinaryPic)
		memcpy(buffer, m_pEventFrameBuffer1_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventGrayPic)
		memcpy(buffer, m_pEventFrameBuffer2_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventAccumulatedPic)
		memcpy(buffer, m_pEventFrameBuffer3_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventSuperimposedPic)
		memcpy(buffer, m_pEventFrameBuffer4_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedBinaryPic)
		memcpy(buffer, m_pEventFrameBuffer5_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountPic)
		memcpy(buffer, m_pEventFrameBuffer6_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedCountPic)
		memcpy(buffer, m_pEventFrameBuffer7_ForUser, CELEX5_PIXELS_NUMBER);

	if (m_bLoopModeEnabled)
		time_stamp = m_lEventFrameTimeStamp_ForUser;
	else
		time_stamp = m_lEventFrameTimeStamp;
}

void CeleX5DataProcessor::getOpticalFlowPicBuffer(unsigned char* buffer, CeleX5::emFullPicType type)
{
	if (type == CeleX5::Full_Optical_Flow_Pic)
		memcpy(buffer, m_pOpticalFrameBuffer1_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::Full_Optical_Flow_Speed_Pic)
		memcpy(buffer, m_pOpticalFrameBuffer2_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::Full_Optical_Flow_Direction_Pic)
		memcpy(buffer, m_pOpticalFrameBuffer3_ForUser, CELEX5_PIXELS_NUMBER);
}

void CeleX5DataProcessor::getOpticalFlowPicBuffer(unsigned char* buffer, std::time_t& time_stamp, CeleX5::emFullPicType type)
{
	if (type == CeleX5::Full_Optical_Flow_Pic)
		memcpy(buffer, m_pOpticalFrameBuffer1_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::Full_Optical_Flow_Speed_Pic)
		memcpy(buffer, m_pOpticalFrameBuffer2_ForUser, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::Full_Optical_Flow_Direction_Pic)
		memcpy(buffer, m_pOpticalFrameBuffer3_ForUser, CELEX5_PIXELS_NUMBER);

	time_stamp = m_lOpticalFrameTimeStamp_ForUser;
}

bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vecData)
{
	vecData = m_vecEventData_ForUser;
	return true;
}

bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vecData, uint64_t& frameNo)
{
	vecData = m_vecEventData_ForUser;
	frameNo = m_uiEventFrameNo;
	return true;
}

bool CeleX5DataProcessor::getEventDataVectorEx(std::vector<EventData> &vecData, std::time_t& time_stamp, bool bDenoised)
{
	vecData = m_vecEventData_ForUser;
	m_vecEventData_ForUser.clear();

	if (bDenoised)
	{
		//todo
	}
	if (m_bLoopModeEnabled)
		time_stamp = m_lEventFrameTimeStamp_ForUser;
	else
		time_stamp = m_lEventFrameTimeStamp;
	return true;
}

//vecData[size -1]: data mode = 1: event / 0: fullpic
void CeleX5DataProcessor::processMIPIData(uint8_t* pData, int dataSize, std::time_t time_stamp_end, vector<IMURawData> imu_data)
{
	//cout << "CeleX5DataProcessor::processData: time_stamp_end = " << time_stamp_end << endl;
	int size = imu_data.size();
	//cout << "CeleX5DataProcessor::processData: imu_data size = " << size << endl;
	if (dataSize == 357001 || dataSize == 1536001)
	{
		for (int i = 0; i < size; i++)
		{
			m_vectorIMU_Raw_data.push_back(imu_data[i]);
			//cout << "--- CeleX5DataProcessor::processData: imu_time_stamp = " << imu_data[i].time_stamp << endl;
		}
	}

	//cout << "CeleX5DataProcessor::processData: data mode ================= " << (int)*(pData + dataSize -1) << endl;
	if (*(pData + dataSize - 1) == 0 /*|| dataSize > 500000*/) //full pic mode: package size = 1536001 = 1280 * 800 * 1.5 + 1
	{
		if (m_bLoopModeEnabled && 
			(m_emCurrentSensorMode == CeleX5::Event_Address_Only_Mode || 
			 m_emCurrentSensorMode == CeleX5::Event_Optical_Flow_Mode ||
			 m_emCurrentSensorMode == CeleX5::Event_Intensity_Mode))
		{
			//cout << "m_uiEventTCounter = " << (m_uiEventTCounter*20)/1000 << endl;
			//cout << "------------------------ " << time_stamp2 - m_lLastPackageTimeStamp << endl;
			m_uiPixelCount = 0;
			m_uiRowCount = 0;
			m_uiEventRowCycleCount = 0;
			m_uiEventTCounter = 0;
			m_uiEventFrameNo++;

			m_vecEventData_ForUser.clear();
			m_vecEventData_ForUser = m_vecEventData;
			m_vecEventData.clear();

			if (m_bIMUModuleEnabled)
			{
				parseIMUData(m_lLastPackageTimeStamp);
			}
			if (m_bFrameModuleEnabled)
			{
				createImage(m_lLastPackageTimeStamp);
			}
			else
			{
				m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
			}
			m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
			//cout << "m_uiEventTCounter = " << m_uiEventTCounter << endl;
		}
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
			parseEventDataFormat0(pData, dataSize);
		}
		else if (m_iMIPIDataFormat == 1)// Format1 (CSR_73 = 1), Package Size: 1024*200*1.75 = 358400 Byte
		{
			parseEventDataFormat1(pData, dataSize);
		}
		else if (m_iMIPIDataFormat == 2)// Format2 (CSR_73 = 2), Package Size: 1024*200*1.75 = 358400 Byte
		{
			parseEventDataFormat2(pData, dataSize);
		}
	}
	m_lLastPackageTimeStamp = time_stamp_end;
}

void CeleX5DataProcessor::disableFrameModule()
{
	m_bFrameModuleEnabled = false;
}

void CeleX5DataProcessor::enableFrameModule()
{
	m_bFrameModuleEnabled = true;
}

bool CeleX5DataProcessor::isFrameModuleEnabled()
{
	return m_bFrameModuleEnabled;
}

void CeleX5DataProcessor::disableIMUModule()
{
	m_bIMUModuleEnabled = false;
}

void CeleX5DataProcessor::enableIMUModule()
{
	m_bIMUModuleEnabled = true;
}

bool CeleX5DataProcessor::isIMUModuleEnabled()
{
	return m_bIMUModuleEnabled;
}

void CeleX5DataProcessor::processFullPicData(uint8_t* pData, int dataSize, std::time_t time_stamp_end)
{
	m_uiPixelCount = 0;
	m_uiRowCount = 0;
	m_uiEventRowCycleCount = 0;
	m_uiEventTCounter = 0;
	m_iLastRowTimeStamp = -1;
	m_iRowTimeStamp = -1;

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

		uint8_t adc1 = value1;
		uint8_t adc2 = value2;

		if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode ||
			m_emCurrentSensorMode == CeleX5::Full_Optical_Flow_S_Mode ||
			m_emCurrentSensorMode == CeleX5::Full_Optical_Flow_M_Mode)
		{
			//m_pEventADCBuffer[i / 3 * 2] = 255 - adc1;
			//m_pEventADCBuffer[i / 3 * 2 + 1] = 255 - adc2;

			m_pEventADCBuffer[index] = 255 - adc1;
			m_pEventADCBuffer[index + 1] = 255 - adc2;
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
			parseIMUData(time_stamp_end);
		}
		if (m_bFrameModuleEnabled)
		{
			createImage(time_stamp_end);
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

void CeleX5DataProcessor::parseEventDataFormat0(uint8_t* pData, int dataSize)
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

void CeleX5DataProcessor::parseEventDataFormat1(uint8_t* pData, int dataSize)
{
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

			//-------- event time stamp --------
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			//row_time_stamp[15:0] = data_28_1[17:2] = value6[1:0] + value5[7:6] + value1[7:0] + value5[5:2]
			m_iRowTimeStamp = ((0x03 & value6) << 14) + ((0xC0 & value5) << 6) + (value1 << 4) + ((0x3C & value5) >> 2);
			//cout << "Format 1 (Package_A): " << "row = " << m_iCurrentRow << ", row_time_stamp = " << m_iRowTimeStamp << endl;
			processMIPIEventTimeStamp();
		}
		else if (dataID1 == 1) //2b'01
		{
			//adc[11:0] = data_28_1[27:16] = value2[7:0] + value6[3:0]
			adc = (value2 << 4) + (0x0F & value6); //for 8-bit: adc = value2
			adc = 4095 - adc;

			//col[10:0] = data_28_1[15:5] = value5[7:6] + value1[7:0] + value5[5]
			col = ((0xC0 & value5) << 3) + (value1 << 1) + ((0x20 & value5) >> 5);

			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				int index = m_iCurrentRow * CELEX5_COL + col;

				m_pEventCountBuffer[index] += 1;
				m_pEventADCBuffer[index] = 255 - value2;
				m_uiPixelCount++;
				m_uiPixelCountForEPS++;

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.adc = adc;
				eventData.t = m_uiEventTCounter;
				eventData.t_increasing = m_uiEventTCounter_Total;
				//--- cal polarity ---
				if (adc > m_pLastADC[index])
					eventData.polarity = 1;
				else if (adc < m_pLastADC[index])
					eventData.polarity = -1;
				else
					eventData.polarity = 0;
				m_pLastADC[index] = adc;
				m_vecEventData.push_back(eventData);
			}
		}
		else if (dataID1 == 0) //2b'00
		{
			//mark[10:0] = data_28_1[27:17] = value2[7:0] + value6[3:1] = 2b'0000 0000 000
			int mark = (value2 << 3) + ((0x0E & value6) >> 1);

			//op_mode[2:0] = data_28_1[16:14] = value6[0] + value5[7:6]
			int op_mode = ((0x01 & value6) << 2) + ((0xC0 & value5) >> 6);
			if (m_bLoopModeEnabled)
				m_emCurrentSensorMode = m_emSensorLoopBMode;
			else
				m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;

			//temperature[11:0] = data_28_1[13:2] = value1[7:0] + value5[5:2]
			//temperature = (value1 << 4) + ((0x3C & value5) >> 2);
			//m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 1 (Package_A): mark = " << mark << ", op_mode = " << op_mode << ", temperature = " << temperature << endl;
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
			//-------- event time stamp --------
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			//row_time_stamp[15:0] = data_28_1[17:2] = value7[5:2] + value3[7:0] + value7[1:0] + value6[7:6]
			m_iRowTimeStamp = ((0x3C & value7) << 10) + (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//cout << "Format 1 (Package_B): " << "row = " << m_iCurrentRow << ", row_time_stamp = " << m_iRowTimeStamp << endl;
			processMIPIEventTimeStamp();
		}
		else if (dataID2 == 1) //2b'01
		{
			//adc[11:0] = data_28_2[27:16] = value4[7:0] + value7[7:4]
			adc = (value4 << 4) + ((0xF0 & value7) >> 4); //for 8-bit: adc = value4
			adc = 4095 - adc;

			//col[10:0] = data_28_2[15:5] = value7[3:2] + value3[7:0]  + value7[1]
			col = ((0x0C & value7) << 7) + (value3 << 1) + ((0x02 & value7) >> 1);

			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				int index = m_iCurrentRow * CELEX5_COL + col;
				m_pEventCountBuffer[index] += 1;
				m_pEventADCBuffer[index] = 255 - value4;
				m_uiPixelCount++;
				m_uiPixelCountForEPS++;

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.adc = adc;
				eventData.t = m_uiEventTCounter;
				eventData.t_increasing = m_uiEventTCounter_Total;
				//--- cal polarity ---
				if (adc > m_pLastADC[index]) //adc is opp
					eventData.polarity = 1;
				else if (adc < m_pLastADC[index])
					eventData.polarity = -1;
				else
					eventData.polarity = 0;
				m_pLastADC[index] = adc;
				m_vecEventData.push_back(eventData);
			}
		}
		else if (dataID2 == 0) //2b'00
		{
			//mark[10:0] = data_28_2[27:17] = value4[7:0] + value7[7:5] = 2b'0000 0000 000
			int mark = (value4 << 3) + ((0xE0 & value7) >> 5);

			//op_mode[2:0] = data_28_2[16:14] = value7[4:2]
			int op_mode = ((0x1C & value7) >> 2);
			if (m_bLoopModeEnabled)
				m_emCurrentSensorMode = m_emSensorLoopBMode;
			else
				m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;

			//temperature[11:0] = data[13:2] = value3[7:0] + value7[1:0] + value6[7:6]
			//temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 1 (Package_B): mark = " << mark << ", op_mode = " << op_mode << ", temperature = " << temperature << endl;
		}
	}
	m_uiPackageTCounter = 0;
}

void CeleX5DataProcessor::parseEventDataFormat2(uint8_t* pData, int dataSize)
{
	//cout << __FUNCTION__ << ": dataSize = " << dataSize << endl;
	if (!m_bLoopModeEnabled)
	{
		if (dataSize != 357001) //358401
		{
			cout << "CeleX5DataProcessor::parseEventDataFormat2: Not a full package: " << dataSize << endl;
			return;
		}
	}
	int index = 0;
	int col = 0;
	int adc = 0;
	int temperature = 0;

	m_emCurrentSensorMode = CeleX5::Event_Address_Only_Mode;

#ifdef _ENABLE_LOG_FILE_
	clock_t begin_t, poll_t;
	begin_t = clock();
#endif // _ENABLE_LOG_FILE_

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
		}
		else if (dataID1 == 0x01) //col data
		{
			//col_addr[10:0] = data_14_1[13:3] = value1[7:0] + value5[5:3]
			col = (value1 << 3) + ((0x38 & value5) >> 3);
			//cout << "--- Package A: " << "col = " << col << endl;
			if (!m_bLoopModeEnabled || (m_bLoopModeEnabled && m_uiEventTCounter >= m_uiEventTCountForRemove))
			{
				if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
				{
					index = m_iCurrentRow * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] += 1;
					}
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;

					EventData eventData;
					eventData.row = m_iCurrentRow;
					eventData.col = col;
					eventData.adc = 0;
					eventData.t = m_uiEventTCounter;
					eventData.t_increasing = m_uiEventTCounter_Total;
					m_vecEventData.push_back(eventData);
				}
			}
		}
		else if (dataID1 == 0x00) // temperature data
		{
			//temperature[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
			//temperature = (value1 << 4) + ((0x3C & value5) >> 2);
			//m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_A): temperature = " << temperature << endl;
		}
		else if (dataID1 == 0x03) //timestamp data
		{
			m_iLastRowTimeStamp = m_iRowTimeStamp;

			//row_time_stamp[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
			m_iRowTimeStamp = (value1 << 4) + ((0x3C & value5) >> 2);
			//cout << "Format 2 (Package_A): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
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
		}
		else if (dataID2 == 0x40) //col data
		{
			//col_addr[10:0] = data_14_2[13:3] = value2[7:0] + value6[3:1]
			col = (value2 << 3) + ((0x0E & value6) >> 1);
			//cout << "--- Package B: " << "col = " << col << endl;
			if (!m_bLoopModeEnabled || (m_bLoopModeEnabled && m_uiEventTCounter >= m_uiEventTCountForRemove))
			{
				if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
				{
					index = m_iCurrentRow * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] += 1;
					}
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;

					EventData eventData;
					eventData.row = m_iCurrentRow;
					eventData.col = col;
					eventData.adc = 0;
					eventData.t = m_uiEventTCounter;
					eventData.t_increasing = m_uiEventTCounter_Total;
					m_vecEventData.push_back(eventData);
				}
			}
		}
		else if (dataID2 == 0x00) //temperature data
		{
			//temperature[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
			//temperature = (value2 << 4) + (0x0F & value6);
			//m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_B): temperature = " << temperature << endl;
		}
		else if (dataID2 == 0xC0) // timestamp data
		{
			//row_time_stamp[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value2 << 4) + (0x0F & value6);
			//cout << "Format 2 (Package_B): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
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
		}
		else if (dataID3 == 0x10) //col data
		{
			//col_addr[10:0] = data_14_3[13:3] = value3[7:0] + value7[1:0] + value6[7]
			col = (value3 << 3) + ((0x03 & value7) << 1) + ((0x80 & value6) >> 7);
			//cout << "--- Package C: " << "col = " << col << endl;
			if (!m_bLoopModeEnabled || (m_bLoopModeEnabled && m_uiEventTCounter >= m_uiEventTCountForRemove))
			{
				if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
				{
					index = m_iCurrentRow * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] += 1;
					}
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;

					EventData eventData;
					eventData.row = m_iCurrentRow;
					eventData.col = col;
					eventData.adc = 0;
					eventData.t = m_uiEventTCounter;
					eventData.t_increasing = m_uiEventTCounter_Total;
					m_vecEventData.push_back(eventData);
				}
			}
		}
		else if (dataID3 == 0x00) //temperature data
		{
			//temperature[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
			//temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_C): temperature = " << temperature << endl;
		}
		else if (dataID3 == 0x30) //timestamp data
		{
			//row_time_stamp[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//cout << "Format 2 (Package_C): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
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
		}
		else if (dataID4 == 0x04) //col data
		{
			//col_addr[10:0] = data_14_4[13:3] = value4[7:0] + value7[7:5]
			col = (value4 << 3) + ((0xE0 & value7) >> 5);
			//cout << "--- Package D: " << "col = " << col << endl;
			if (!m_bLoopModeEnabled || (m_bLoopModeEnabled && m_uiEventTCounter >= m_uiEventTCountForRemove))
			{
				if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
				{
					index = m_iCurrentRow * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] += 1;
					}
					m_uiPixelCount++;
					m_uiPixelCountForEPS++;

					EventData eventData;
					eventData.row = m_iCurrentRow;
					eventData.col = col;
					eventData.adc = 0;
					eventData.t = m_uiEventTCounter;
					eventData.t_increasing = m_uiEventTCounter_Total;
					m_vecEventData.push_back(eventData);
				}
			}
		}
		else if (dataID4 == 0x00) //temperature data
		{
			//temperature[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
			//temperature = (value4 << 4) + ((0xF0 & value7) >> 4);
			//m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_D): temperature = " << temperature << endl;
		}
		else if (dataID4 == 0x0C) //timestamp data
		{
			//row_time_stamp[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value4 << 4) + ((0xF0 & value7) >> 4);
			//cout << "Format 2 (Package_D): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
		}
	}
	m_uiPackageTCounter = 0;

#ifdef _ENABLE_LOG_FILE_
	poll_t = clock();
	m_ofLogFile << "  Elapsed Time: " << (poll_t - begin_t) << " (ms)" << std::endl;
#endif
}

#define gravity 9.80665
#define accResolution /*(2 * gravity / (std::pow(2.0,15) -1))*/ 0.000598569
#define resolution /*(250.0 / (std::pow(2.0, 15)-1) / 180 * CV_PI)*/ 0.000133162
#define magResolution /*(4800.0 / (std::pow(2.0, 15)-1))*/ 0.146489
void CeleX5DataProcessor::parseIMUData(std::time_t time_stamp)
{
	int end = 0;
	//cout << "-----frame time_stamp-----" << time_stamp << ", size = " << m_vectorIMU_Raw_data.size() << endl;

	for (int i = 0; i < m_vectorIMU_Raw_data.size(); i++)
	{
		IMUData imuData;
		IMURawData imu_raw_data = m_vectorIMU_Raw_data[i];
		imuData.frameNo = 0;
		imuData.time_stamp = imu_raw_data.time_stamp;

		if (imuData.time_stamp < time_stamp /*&& imuData.time_stamp > time_stamp - 30*/)
		{
			//cout << "imuData.time_stamp: " << imuData.time_stamp << endl;

			int16_t x_ACC = (imu_raw_data.imu_data[0] << 8) + imu_raw_data.imu_data[1];
			int16_t y_ACC = (imu_raw_data.imu_data[2] << 8) + imu_raw_data.imu_data[3];
			int16_t z_ACC = (imu_raw_data.imu_data[4] << 8) + imu_raw_data.imu_data[5];
			//cout << "x_ACC = " << x_ACC << ", y_ACC = " << y_ACC << ", z_ACC = " << z_ACC << endl;
			imuData.x_ACC = y_ACC * accResolution; //x_ACC * accResolution
			imuData.y_ACC = x_ACC * accResolution; //y_ACC * accResolution
			imuData.z_ACC = -z_ACC * accResolution;
			//cout << "x_ACC = " << imuData.x_ACC << ", y_ACC = " << imuData.y_ACC << ", z_ACC = " << imuData.z_ACC << endl;

			int16_t x_TEMP = (imu_raw_data.imu_data[6] << 8) + imu_raw_data.imu_data[7];
			imuData.x_TEMP = x_TEMP;
			imuData.x_TEMP = 21 + (x_TEMP / 333.87);
			//cout << "x_TEMP = " << imuData.x_TEMP << endl;

			int16_t x_GYROS = (imu_raw_data.imu_data[8] << 8) + imu_raw_data.imu_data[9];
			int16_t y_GYROS = (imu_raw_data.imu_data[10] << 8) + imu_raw_data.imu_data[11];
			int16_t z_GYROS = (imu_raw_data.imu_data[12] << 8) + imu_raw_data.imu_data[13];
			//cout << "x_GYROS = " << x_GYROS << ", y_GYROS = " << y_GYROS << ", z_GYROS = " << z_GYROS << endl;
			imuData.x_GYROS = y_GYROS * resolution; //x_GYROS * resolution
			imuData.y_GYROS = -x_GYROS * resolution; //y_GYROS * resolution
			imuData.z_GYROS = z_GYROS * resolution;
			//cout << "x_GYROS = " << imuData.x_GYROS << ", y_GYROS = " << imuData.y_GYROS << ", z_GYROS = " << imuData.z_GYROS << endl;

			int16_t x_MAG = imu_raw_data.imu_data[14] + (imu_raw_data.imu_data[15] << 8);
			int16_t y_MAG = imu_raw_data.imu_data[16] + (imu_raw_data.imu_data[17] << 8);
			int16_t z_MAG = imu_raw_data.imu_data[18] + (imu_raw_data.imu_data[19] << 8);
			//cout << "z_MAG_19 = " << (int)imu_raw_data.imu_data[19] << ", z_MAG_20 = " << int(imu_raw_data.imu_data[20]) << endl;
			//cout << "x_MAG = " << x_MAG << ", y_MAG = " << y_MAG << ", z_MAG = " << z_MAG << endl;
			imuData.x_MAG = x_MAG * magResolution; 
			imuData.y_MAG = y_MAG * magResolution; 
			imuData.z_MAG = z_MAG * magResolution;
			//cout << "x_MAG = " << imuData.x_MAG << ", y_MAG = " << imuData.y_MAG << ", z_MAG = " << imuData.z_MAG << endl;

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
		auto iter = m_vectorIMU_Raw_data.begin();
		m_vectorIMU_Raw_data.erase(iter);
	}
}

CX5SensorDataServer *CeleX5DataProcessor::getSensorDataServer()
{
	return m_pCX5Server;
}

CeleX5ProcessedData *CeleX5DataProcessor::getProcessedData()
{
	return m_pCX5ProcessedData;
}

void CeleX5DataProcessor::checkIfShowImage()
{
	if (EventShowByRowCycle == m_emEventShowType) //show by row count
	{
		if (m_uiEventRowCycleCount >= m_uiEventRowCycleCountForShow && m_uiRowCount >= 100 * m_uiEventRowCycleCountForShow)
		{
			//cout << "-------------- m_uiPixelCount = " << m_uiPixelCount << ", m_uiRowCount = " << m_uiRowCount << endl;
			m_uiPixelCount = 0;
			m_uiRowCount = 0;
			m_uiEventRowCycleCount = 0;
			m_uiEventTCounter = 0;
			m_uiEventFrameNo++;

			//cout << __FUNCTION__ << ": m_vecEventData.size() = " << m_vecEventData.size() << endl;
			m_vecEventData_ForUser.clear();
			m_vecEventData_ForUser = m_vecEventData;
			m_vecEventData.clear();
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
	}
	else if (EventShowByTime == m_emEventShowType) //show by time
	{
		if (m_uiEventTCounter >= m_uiEventTCountForShow)
		{
			//cout << "m_uiPixelCount = " << m_uiPixelCount << endl;
			m_uiPixelCount = 0;
			m_uiRowCount = 0;
			m_uiEventRowCycleCount = 0;
			m_uiEventTCounter = 0;
			m_uiEventFrameNo++;
			if (m_lLastPackageTimeStamp == 0)
			{
				m_lEventFrameTimeStamp = 0;
			}
			else
			{
				m_lEventFrameTimeStamp = m_lLastPackageTimeStamp + m_uiPackageTCounter * m_uiCurrentEventTUnit / 1000; //m_uiPackageTCounter * m_uiCurrentEventTUnit / 1000
			}
			//cout << __FUNCTION__ << "m_lLastPackageTimeStamp = " << m_lLastPackageTimeStamp << endl;
			//cout << __FUNCTION__ << ": m_lEventFrameTimeStamp = " << m_lEventFrameTimeStamp << endl;
			//cout << __FUNCTION__ << ": m_uiPackageTCounter = " << m_uiPackageTCounter << endl;
			if (m_bIMUModuleEnabled)
			{
				parseIMUData(m_lEventFrameTimeStamp);
			}

			m_vecEventData_ForUser.clear();
			m_vecEventData_ForUser = m_vecEventData;
			m_vecEventData.clear();

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
	}
	else if (EventShowByCount == m_emEventShowType) //show by event count
	{
		if (m_uiPixelCount >= m_uiEventCountForShow)
		{
			m_uiPixelCount = 0;
			m_uiRowCount = 0;
			m_uiEventRowCycleCount = 0;
			m_uiEventTCounter = 0;
			m_uiEventFrameNo++;

			//cout << __FUNCTION__ << ": m_vecEventData.size() = " << m_vecEventData.size() << endl;
			m_vecEventData_ForUser.clear();
			m_vecEventData_ForUser = m_vecEventData;
			m_vecEventData.clear();

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
	}
}

bool CeleX5DataProcessor::createImage(std::time_t time_stamp_end)
{
	unsigned char* pFullPic = m_pCX5ProcessedData->getFullPicBuffer();
	unsigned char* pOpticalFlowPic = m_pCX5ProcessedData->getOpticalFlowPicBuffer(CeleX5::Full_Optical_Flow_Pic);
	unsigned char* pOpticalFlowSpeedPic = m_pCX5ProcessedData->getOpticalFlowPicBuffer(CeleX5::Full_Optical_Flow_Speed_Pic);
	unsigned char* pOpticalFlowDirectionPic = m_pCX5ProcessedData->getOpticalFlowPicBuffer(CeleX5::Full_Optical_Flow_Direction_Pic);
	unsigned char* pEventBinaryPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventBinaryPic);
	unsigned char* pEventGrayPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventGrayPic);
	unsigned char* pEventAccumulatedPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventAccumulatedPic);
	unsigned char* pEventDenoisedBinaryPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventDenoisedBinaryPic);
	unsigned char* pEventSuperimposedPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventSuperimposedPic);
	unsigned char* pEventCountPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventCountPic);
	unsigned char* pEventDenoisedCountPic = m_pCX5ProcessedData->getEventPicBuffer(CeleX5::EventDenoisedCountPic);

	m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
	m_pCX5ProcessedData->setEventDataVector(m_vecEventData_ForUser);
	if (m_bLoopModeEnabled)
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
	else
	{
		m_pCX5ProcessedData->setLoopNum(-1);
	}

	int value = 0, value1 = 0, index = 0;
	int row, col, index1, index2, index3, index4;
	if (m_emCurrentSensorMode == CeleX5::Event_Address_Only_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			switch (m_iRotateType)
			{
			case 0:
				index = CELEX5_PIXELS_NUMBER - i - 1;
				break;
			case 1:
				index = (i / CELEX5_COL * CELEX5_COL + (CELEX5_COL - i % CELEX5_COL - 1));
				break;
			case 2:
				index = ((CELEX5_ROW - i / CELEX5_COL - 1)* CELEX5_COL + i % CELEX5_COL);
				break;
			case 3:
				index = i;
				break;
			}
			if (m_pEventCountBuffer[i] > 0)
			{
				value = 255;
				value1 = m_pEventCountBuffer[i] * m_uiEventCountStep;
			}
			else
			{
				value = value1 = 0;
			}
			pEventBinaryPic[index] = value;
			pEventDenoisedCountPic[index] = pEventCountPic[index] = value1;
			//--- denoised pic ---
			int score = 0;
			if (m_pEventCountBuffer[i] > 0)
			{
				if (calculateDenoiseScore(m_pEventCountBuffer, i) > 0)
				{
					score = 255;
				}
			}
			if (score == 0)
			{
				pEventDenoisedCountPic[index] = 0;
			}
			pEventDenoisedBinaryPic[index] = score;
		}
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Optical_Flow_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			switch (m_iRotateType)
			{
			case 0:
				index = CELEX5_PIXELS_NUMBER - i - 1;
				break;
			case 1:
				index = (i / CELEX5_COL * CELEX5_COL + (CELEX5_COL - i % CELEX5_COL - 1));
				break;
			case 2:
				index = ((CELEX5_ROW - i / CELEX5_COL - 1)* CELEX5_COL + i % CELEX5_COL);
				break;
			case 3:
				index = i;
				break;
			}

			if (m_pEventADCBuffer[i] > 0)
			{
				pOpticalFlowPic[index] = m_pEventADCBuffer[i];
				pEventBinaryPic[index] = 255;
			}
			else
			{
				pOpticalFlowPic[index] = 0;
				pEventBinaryPic[index] = 0;
			}
		}
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Intensity_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			switch (m_iRotateType)
			{
			case 0:
				index = CELEX5_PIXELS_NUMBER - i - 1;
				break;
			case 1:
				index = (i / CELEX5_COL * CELEX5_COL + (CELEX5_COL - i % CELEX5_COL - 1));
				break;
			case 2:
				index = ((CELEX5_ROW - i / CELEX5_COL - 1)* CELEX5_COL + i % CELEX5_COL);
				break;
			case 3:
				index = i;
				break;
			}
			if (m_pEventADCBuffer[i] > 0)
			{
				value = m_pEventADCBuffer[i] - m_pFpnBuffer[i];
				value1 = m_pEventCountBuffer[i] * m_uiEventCountStep;
				if (value < 0)
					value = 0;
				if (value > 255)
					value = 255;
				pEventBinaryPic[index] = 255;
				pEventGrayPic[index] = value;
				pEventAccumulatedPic[index] = value; //don't need to clear
				pEventSuperimposedPic[index] = pEventBinaryPic[index] > 0 ? 255 : value;//superimpose
				pEventCountPic[index] = value1;
			}
			else
			{
				pEventBinaryPic[index] = 0;
				pEventGrayPic[index] = 0;
				pEventSuperimposedPic[index] = pEventAccumulatedPic[index];
				pEventCountPic[index] = 0;
			}
		}
	}
	else if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			switch (m_iRotateType)
			{
			case 0:
				index = CELEX5_PIXELS_NUMBER - i - 1;
				break;
			case 1:
				index = (i / CELEX5_COL * CELEX5_COL + (CELEX5_COL - i % CELEX5_COL - 1));
				break;
			case 2:
				index = ((CELEX5_ROW - i / CELEX5_COL - 1)* CELEX5_COL + i % CELEX5_COL);
				break;
			case 3:
				index = i;
				break;
			}

			row = i / CELEX5_COL;
			col = i % CELEX5_COL;
			if (row == 0 || row == 799 || col == 0 || col == 1279)
				continue;
			index1 = (row - 1)*CELEX5_COL + col;
			index2 = row*CELEX5_COL + col - 1;
			index3 = row*CELEX5_COL + col + 1;
			index4 = (row + 1)*CELEX5_COL + col;

			if (m_pEventADCBuffer[i] < 255 && m_pEventADCBuffer[i] > 0)
				value = m_pEventADCBuffer[i] - m_pFpnBuffer[i];//Subtract FPN
			else if (m_pEventADCBuffer[index1] < 255 && m_pEventADCBuffer[index1] > 0)
				value = m_pEventADCBuffer[index1] - m_pFpnBuffer[index1];
			else if (m_pEventADCBuffer[index2] < 255 && m_pEventADCBuffer[index2] > 0)
				value = m_pEventADCBuffer[index2] - m_pFpnBuffer[index2];
			else if (m_pEventADCBuffer[index3] < 255 && m_pEventADCBuffer[index3] > 0)
				value = m_pEventADCBuffer[index3] - m_pFpnBuffer[index3];
			else if (m_pEventADCBuffer[index4] < 255 && m_pEventADCBuffer[index4] > 0)
				value = m_pEventADCBuffer[index4] - m_pFpnBuffer[index4];
			else
				value = m_pEventADCBuffer[i];
			if (value < 0)
				value = 0;
			if (value > 255)
				value = 255;
			pFullPic[index] = value;
		}
	}
	else if (m_emCurrentSensorMode == CeleX5::Full_Optical_Flow_S_Mode ||
		     m_emCurrentSensorMode == CeleX5::Full_Optical_Flow_M_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			switch (m_iRotateType)
			{
			case 0:
				index = CELEX5_PIXELS_NUMBER - i - 1;
				break;
			case 1:
				index = (i / CELEX5_COL * CELEX5_COL + (CELEX5_COL - i % CELEX5_COL - 1));
				break;
			case 2:
				index = ((CELEX5_ROW - i / CELEX5_COL - 1)* CELEX5_COL + i % CELEX5_COL);
				break;
			case 3:
				index = i;
				break;
			}
			value = m_pEventADCBuffer[i];
			//cout << "m_pEventADCBuffer: " << value << endl;
			if (value == 255)
				value = 0;
			pOpticalFlowPic[index] = value;
			calDirectionAndSpeed(i, index, m_pEventADCBuffer, pOpticalFlowSpeedPic, pOpticalFlowDirectionPic);
		}
	}
	for (int j = 0; j < CELEX5_PIXELS_NUMBER; ++j)
	{
		m_pEventCountBuffer[j] = 0;
		m_pEventADCBuffer[j] = 0;
	}

	if (m_emCurrentSensorMode == CeleX5::Full_Optical_Flow_S_Mode ||
		m_emCurrentSensorMode == CeleX5::Full_Optical_Flow_M_Mode)
	{
		memcpy(m_pOpticalFrameBuffer1_ForUser, pOpticalFlowPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pOpticalFrameBuffer2_ForUser, pOpticalFlowSpeedPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pOpticalFrameBuffer3_ForUser, pOpticalFlowDirectionPic, CELEX5_PIXELS_NUMBER);

		m_lOpticalFrameTimeStamp_ForUser = time_stamp_end;
	}
	else if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode)
	{
		memcpy(m_pFullFrameBuffer_ForUser, pFullPic, CELEX5_PIXELS_NUMBER);
		m_lFullFrameTimeStamp_ForUser = time_stamp_end;
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Address_Only_Mode)
	{
		memcpy(m_pEventFrameBuffer1_ForUser, pEventBinaryPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer5_ForUser, pEventDenoisedBinaryPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer6_ForUser, pEventCountPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer7_ForUser, pEventDenoisedCountPic, CELEX5_PIXELS_NUMBER);
		m_lEventFrameTimeStamp_ForUser = time_stamp_end;
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Intensity_Mode)
	{
		memcpy(m_pEventFrameBuffer1_ForUser, pEventBinaryPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer2_ForUser, pEventGrayPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer3_ForUser, pEventAccumulatedPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer4_ForUser, pEventSuperimposedPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer6_ForUser, pEventCountPic, CELEX5_PIXELS_NUMBER);
		m_lEventFrameTimeStamp_ForUser = time_stamp_end;
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Optical_Flow_Mode)
	{
		memcpy(m_pOpticalFrameBuffer1_ForUser, pOpticalFlowPic, CELEX5_PIXELS_NUMBER);
		memcpy(m_pEventFrameBuffer1_ForUser, pEventBinaryPic, CELEX5_PIXELS_NUMBER);
		m_lEventFrameTimeStamp_ForUser = time_stamp_end;
	}
	return true;
}

// normalize to 0~255
unsigned int CeleX5DataProcessor::normalizeADC(unsigned int adc)
{
	int brightness = adc;
	if (adc < 0)
		brightness = 0;
	else if (adc > 4096)
		brightness = 255;
	else
		brightness = 255 * adc / 4096 - 0;
	return brightness;
}

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
		if (m_strFpnFilePath.empty())
		{
			XBase base;
			std::string filePath = base.getApplicationDirPath();
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
	return true;
}

void CeleX5DataProcessor::generateFPN(std::string filePath)
{
	m_bIsGeneratingFPN = true;
	m_iFpnCalculationTimes = FPN_CALCULATION_TIMES;
	m_strFpnFilePath = filePath;
}

CeleX5::CeleX5Mode CeleX5DataProcessor::getSensorFixedMode()
{
	return m_emSensorFixedMode;
}

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

void CeleX5DataProcessor::setSensorFixedMode(CeleX5::CeleX5Mode mode)
{
	m_emCurrentSensorMode = m_emSensorFixedMode = mode;
	m_uiEventFrameNo = 0;
}

void CeleX5DataProcessor::setSensorLoopMode(CeleX5::CeleX5Mode mode, int loopNum)
{
	if (1 == loopNum)
		m_emSensorLoopAMode = mode;
	else if (2 == loopNum)
		m_emSensorLoopBMode = mode;
	else if (3 == loopNum)
		m_emSensorLoopCMode = mode;
}

void CeleX5DataProcessor::setLoopModeEnabled(bool enable)
{
	m_bLoopModeEnabled = enable;
	m_uiEventFrameNo = 0;
}

void CeleX5DataProcessor::setISOLevel(uint32_t value)
{
	m_uiISOLevel = value;
	//--- load default fpn file ---
	XBase base;
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

void CeleX5DataProcessor::setMIPIDataFormat(int format)
{
	m_iMIPIDataFormat = format;
}

void CeleX5DataProcessor::setEventFrameTime(uint32_t value, uint32_t clock) //unit: microsecond
{
	m_uiEventFrameTime = value;
	m_uiCurrentEventTUnit = m_uiEventTUnitList[clock / 10 - 1];
	m_uiEventTCountForShow = value / m_uiCurrentEventTUnit;
	m_uiEventTCountForEPS = 1000000 / m_uiCurrentEventTUnit;
	cout << __FUNCTION__ << ": m_uiEventTCountForShow = " << m_uiEventTCountForShow << endl;
	cout << __FUNCTION__ << ": m_uiEventTCountForEPS = " << m_uiEventTCountForEPS << endl;
}
uint32_t CeleX5DataProcessor::getEventFrameTime()
{
	return m_uiEventFrameTime;
}

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

EventShowType CeleX5DataProcessor::getEventShowMethod()
{
	return m_emEventShowType;
}

void CeleX5DataProcessor::setEventFrameStartPos(uint32_t value)
{
	m_uiEventTCountForRemove = value * m_uiEventTCountForShow * 1000 / m_uiEventFrameTime;
	cout << __FUNCTION__ << ": m_uiEventTCountForRemove = " << m_uiEventTCountForRemove << endl;
}

//int CeleX5DataProcessor::calculateDenoiseScore(unsigned char* pBuffer, unsigned int pos)	//8 neighbors
//{
//	if (NULL == pBuffer)
//	{
//		return 255;
//	}
//	int row = pos / CELEX5_COL;
//	int col = pos % CELEX5_COL;
//
//	int count1 = 0;
//	int count2 = 0;
//	for (int i = row - 1; i < row + 2; ++i) //8 points
//	{
//		for (int j = col - 1; j < col + 2; ++j)
//		{
//			int index = i * CELEX5_COL + j;
//			if (index < 0 || index == pos || index >= CELEX5_PIXELS_NUMBER)
//				continue;
//			if (pBuffer[index] > 0)
//				++count1;
//			else
//				++count2;
//		}
//	}
//	if (count1 >= count2)
//		return 255;
//	else
//		return 0;
//}

int CeleX5DataProcessor::calculateDenoiseScore(unsigned char* pBuffer, unsigned int pos)	//4 neighbours
{
	if (NULL == pBuffer)
	{
		return 255;
	}
	int row = pos / CELEX5_COL;
	int col = pos % CELEX5_COL;

	int count1 = 0;
	int count2 = 0;
	int index1 = (row - 1)*CELEX5_COL + col;
	int index2 = row*CELEX5_COL + col - 1;
	int index3 = row*CELEX5_COL + col + 1;
	int index4 = (row + 1)*CELEX5_COL + col;
	int aa[4] = { index1, index2, index3, index4 };
	for (int i = 0; i < 4; ++i)
	{
		if (aa[i] < 0 || aa[i] >= CELEX5_PIXELS_NUMBER)
			continue;
		if (pBuffer[aa[i]] > 0)
			++count1;
		else
			++count2;
	}
	if (count1 >= count2)
		return 255;
	else
		return 0;
}

int CeleX5DataProcessor::calMean(unsigned char* pBuffer, unsigned int pos)
{
	if (NULL == pBuffer)
	{
		return 255;
	}
	int row = pos / CELEX5_COL;
	int col = pos % CELEX5_COL;
	int value = 0;
	int index = 0;
	for (int i = row - 1; i < row + 2; ++i) //8 points
	{
		for (int j = col - 1; j < col + 2; ++j)
		{
			int index = i * CELEX5_COL + j;
			if (index < 0 || index == pos || index >= CELEX5_PIXELS_NUMBER)
				continue;
			value += pBuffer[index];
			index++;
		}
	}
	if (index > 0)
		return value / index;
	else
		return 255;
}

void CeleX5DataProcessor::calDirectionAndSpeed(int i, int j, unsigned char* pBuffer, unsigned char* &speedBuffer, unsigned char* &dirBuffer)
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

void CeleX5DataProcessor::setRotateType(int type)
{
	m_iRotateType = type;
}

int CeleX5DataProcessor::getRotateType()
{
	return m_iRotateType;
}

void CeleX5DataProcessor::setEventCountStep(uint32_t step)
{
	m_uiEventCountStep = step;
}

uint32_t CeleX5DataProcessor::getEventCountStep()
{
	return m_uiEventCountStep;
}

int CeleX5DataProcessor::getIMUData(std::vector<IMUData>& data)
{
	m_vectorIMUData.swap(data);
	vector<IMUData> tempVec;
	m_vectorIMUData.swap(tempVec);
	return data.size();
}

void CeleX5DataProcessor::saveFullPicRawData()
{
	//cout << "----- CeleX5DataProcessor::saveFullPicRawData" << endl;
	m_bSaveFullPicRawData = true;
}

void CeleX5DataProcessor::resetTimestamp()
{
	m_vecEventData.clear();
	m_vecEventData_ForUser.clear();
	m_vectorIMUData.clear();
	m_vectorIMU_Raw_data.clear();

	m_iLastRowTimeStamp = -1;
	m_iRowTimeStamp = -1;
	m_uiEventTCounter = 0;
	m_uiEventTCounter_Total = 0;
	m_uiEventNumberEPS = 0;
	m_uiPixelCountForEPS = 0;
	m_uiEventTCounter_EPS = 0;
}

uint32_t CeleX5DataProcessor::getEventRate()
{
	return m_uiEventNumberEPS;
}

void CeleX5DataProcessor::saveFullPicRawData(uint8_t* pData)
{
	//cout << "CeleX5DataProcessor::saveFullPicRawData" << endl;
	ofstream ofFullPic;
	ofFullPic.open("full_pic_raw_data.txt");

	int dataSize = 1536001;
	int index = 0;
	for (int i = 0; i < dataSize - 2; i += 3)
	{
		uint8_t value1 = *(pData + i);
		uint8_t value2 = *(pData + i + 1);
		uint8_t value3 = *(pData + i + 2);

		uint16_t adc11 = (value1 << 4) + (0x0F & value3);
		uint16_t adc22 = (value2 << 4) + ((0xF0 & value3) >> 4);

		ofFullPic << adc11 << "  " << adc22 << "  "; 
		//ofFullPic << 255 - (int)value1 << "  " << 255 - (int)value2 << "  ";
		index += 2;
		if (index % 1280 == 0)
			ofFullPic << endl;
	}
	ofFullPic.close();
	m_bSaveFullPicRawData = false;
}
