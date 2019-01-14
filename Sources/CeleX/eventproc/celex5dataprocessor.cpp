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

unsigned char* pFrameBuffer1 = new unsigned char[CELEX5_PIXELS_NUMBER];
unsigned char* pFrameBuffer2 = new unsigned char[CELEX5_PIXELS_NUMBER];
unsigned char* pFrameBuffer3 = new unsigned char[CELEX5_PIXELS_NUMBER];
unsigned char* pFrameBuffer4 = new unsigned char[CELEX5_PIXELS_NUMBER];
unsigned char* pFrameBuffer5 = new unsigned char[CELEX5_PIXELS_NUMBER];
unsigned char* pFrameBuffer6 = new unsigned char[CELEX5_PIXELS_NUMBER];

vector<EventData> g_vecEventData;

CeleX5DataProcessor::CeleX5DataProcessor()
	: m_uiPixelCount(0)
	, m_uiEventTCounter(0)
	, m_uiEventTCountForShow(1500)
	, m_uiEventCountForShow(100000)
	, m_uiEventRowCycleCountForShow(6)
	, m_iLastRow(-1)
	, m_iCurrentRow(-1)
	, m_uiRowCount(0)
	, m_bIsGeneratingFPN(false)
	, m_iFpnCalculationTimes(-1)
	, m_bLoopModeEnabled(false)
	, m_uiISOLevel(3)
	, m_iLastRowTimeStamp(-1)
	, m_iRowTimeStamp(-1)
	, m_iMIPIDataFormat(2)
	, m_emSensorFixedMode(CeleX5::Event_Address_Only_Mode)
	, m_emSensorLoopAMode(CeleX5::Event_Address_Only_Mode)
	, m_emSensorLoopBMode(CeleX5::Full_Picture_Mode)
	, m_emSensorLoopCMode(CeleX5::Full_Optical_Flow_S_Mode)
	, m_uiEventFrameTime(30000)
	, m_uiEventRowCycleCount(0)
	, m_emEventShowType(EventShowByTime)
	, m_uiEventTUnitList{10, 25, 22, 25, 20, 17, 14, 25, 22, 20, 10, 10, 10, 10, 10, 10}
	, m_uiEventFrameNo(0)
{
	m_pFPGADataReader = new CeleX5DataReader;
	m_pEventADCBuffer = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventCountBuffer = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pLastADC = new uint16_t[CELEX5_PIXELS_NUMBER];

	m_pFpnGenerationBuffer = new long[CELEX5_PIXELS_NUMBER];
	m_pFpnBuffer = new int[CELEX5_PIXELS_NUMBER];

	for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
	{
		m_pEventADCBuffer[i] = 0;
		m_pEventCountBuffer[i] = 0;
		m_pLastADC[i] = 0;
		//
		m_pFpnGenerationBuffer[i] = 0;
		m_pFpnBuffer[i] = 0;
	}

	m_pCX5ProcessedData = new CeleX5ProcessedData;
	m_pCX5Server = new CX5SensorDataServer;
	m_pCX5Server->setCX5SensorData(m_pCX5ProcessedData);

	//XBase base;
	//std::string filePath = base.getApplicationDirPath();
	//filePath += "\\FPN_Gain2.txt";
	//setFpnFile(filePath);

#ifdef _ENABLE_LOG_FILE_
	if (!m_ofLogFile.is_open())
		m_ofLogFile.open("log_file_time.txt");
#endif
}

CeleX5DataProcessor::~CeleX5DataProcessor()
{
	if (m_pFPGADataReader) delete m_pFPGADataReader;
	if (m_pCX5ProcessedData) delete m_pCX5ProcessedData;
	if (m_pCX5Server) delete m_pCX5Server;

	if (m_pEventCountBuffer) delete[] m_pEventCountBuffer;
	if (m_pEventADCBuffer) delete[] m_pEventADCBuffer;
	if (m_pFpnGenerationBuffer) delete[] m_pFpnGenerationBuffer;
	if (m_pFpnBuffer) delete[] m_pFpnBuffer;
}

void CeleX5DataProcessor::getFullPicBuffer(unsigned char* buffer)
{
	memcpy(buffer, pFrameBuffer1, CELEX5_PIXELS_NUMBER);
}

void CeleX5DataProcessor::getEventPicBuffer(unsigned char* buffer, CeleX5::emEventPicType type)
{
	if (type == CeleX5::EventBinaryPic)
		memcpy(buffer, pFrameBuffer1, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventGrayPic)
		memcpy(buffer, pFrameBuffer2, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventAccumulatedPic)
		memcpy(buffer, pFrameBuffer3, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventSuperimposedPic)
		memcpy(buffer, pFrameBuffer4, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventDenoisedBinaryPic)
		memcpy(buffer, pFrameBuffer5, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::EventCountPic)
		memcpy(buffer, pFrameBuffer6, CELEX5_PIXELS_NUMBER);
}

void CeleX5DataProcessor::getOpticalFlowPicBuffer(unsigned char* buffer, CeleX5::emFullPicType type)
{
	if (type == CeleX5::Full_Optical_Flow_Pic)
		memcpy(buffer, pFrameBuffer2, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::Full_Optical_Flow_Speed_Pic)
		memcpy(buffer, pFrameBuffer3, CELEX5_PIXELS_NUMBER);
	else if (type == CeleX5::Full_Optical_Flow_Direction_Pic)
		memcpy(buffer, pFrameBuffer4, CELEX5_PIXELS_NUMBER);
}

bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vector)
{
	vector = g_vecEventData;
	return true;
}

bool CeleX5DataProcessor::getEventDataVector(std::vector<EventData> &vector, uint64_t& frameNo)
{
	vector = g_vecEventData;
	frameNo = m_uiEventFrameNo;
	return true;
}

void CeleX5DataProcessor::processData(unsigned char* data, long length)
{
	if (!data)
	{
		return;
	}
	//--------------- Align data ---------------
	long i = 0;
	//if (!bAligned)
	//{
	//	while (i + 1 < length)
	//	{
	//		if ((data[i] & 0x80) > 0 && (data[i + 1] & 0x80) == 0x00)
	//		{
	//			i = i - 3;
	//			// init sRow and sT
	//			processEvent(&data[i]);
	//			bAligned = true;
	//			break; // aligned
	//		}
	//		++i;
	//	}
	//}
	//for (i = i + EVENT_SIZE; i + 11 < length; i = i + EVENT_SIZE)
	for (; i + EVENT_SIZE <= length; i = i + EVENT_SIZE)
	{
		bool isSpecialEvent = !(processEvent(&data[i]));
		if (isSpecialEvent)
		{
			if (m_bIsGeneratingFPN)
			{
				generateFPNimpl();
			}
			else
			{
				createImage();
				m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
			}
		}
	}
}

bool CeleX5DataProcessor::processEvent(unsigned char* data)
{
	if (m_pFPGADataReader->isSpecialEvent(data))
	{
		cout << "m_uiPixelCount = " << m_uiPixelCount << endl;
		m_uiPixelCount = 0;
		return false;

		//cout << "Mode = " << (int)m_emCurrentSensorMode << ", Pixel Count = " << m_uiPixelCount << endl;
		if (m_bLoopModeEnabled)
		{
			if (m_uiPixelCount > 0)
			{
				m_uiPixelCount = 0;
				return false;
			}
		}
		else
		{
			m_uiPixelCount = 0;
			return false;
		}
	}
	else
	{
		if (m_emSensorFixedMode == CeleX5::Event_Address_Only_Mode ||
			m_emSensorFixedMode == CeleX5::Event_Optical_Flow_Mode ||
			m_emSensorFixedMode == CeleX5::Event_Intensity_Mode)
		{
			if (m_pFPGADataReader->isRowEvent(data))
			{
				//cout << "RowEvent: row = " << m_pFPGADataReader->row() << endl;
				return processRowEvent(data);
			}
			else if (m_pFPGADataReader->isColumnEvent(data))
			{
				//cout << "ColEvent" << endl;
				return processColEvent(data);
			}
			//else if (m_pFPGADataReader->isRowEventEx(data))
			//{
			//	//m_pFPGADataReader->parseRowEventEx(data);
			//	//cout << "RowEvent: row = " << m_pFPGADataReader->row() << endl;
			//	//return processRowEvent(data);
			//}
		}
		else
		{
			if (m_uiPixelCount + 3 < CELEX5_PIXELS_NUMBER)
			{
				m_pEventADCBuffer[m_uiPixelCount] = 255 - (int)data[0];
				m_pEventADCBuffer[m_uiPixelCount + 1] = 255 - (int)data[1];
				m_pEventADCBuffer[m_uiPixelCount + 2] = 255 - (int)data[2];
				m_pEventADCBuffer[m_uiPixelCount + 3] = 255 - (int)data[3];
			}
			m_uiPixelCount += 4;
			//cout << (int)m_pEventADCBuffer[m_uiPixelCount] << " ";
		}
	}

	return true;
}

//vecData[size -1]: data mode = 1: event / 0: fullpic
void CeleX5DataProcessor::processMIPIData(uint8_t* pData, int dataSize)
{
	//cout << "CeleX5DataProcessor::processData: vecData size = " << dataSize << endl;
	//cout << "CeleX5DataProcessor::processData: data mode ================= " << (int)*(pData + dataSize -1) << endl;
	if (*(pData + dataSize - 1) == 0 /*|| dataSize > 500000*/) //full pic mode: package size = 1536001 = 1280 * 800 * 1.5 + 1
	{
		uint8_t mode = 0xFF & *pData;
		if (mode == 0x60 || mode == 0x80 || mode == 0xA0 || mode == 0xC0 || mode == 0xE0)
		{
			m_emCurrentSensorMode = CeleX5::CeleX5Mode((0xE0 & *pData) >> 5);
			//cout << "CeleX5DataProcessor::processData: m_emCurrentSensorMode = " << (int)m_emCurrentSensorMode << endl;
			processFullPicData(pData, dataSize);
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
}

void CeleX5DataProcessor::processFullPicData(uint8_t* pData, int dataSize)
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

	int index = 0;
#ifdef _OPENMP
#pragma omp parallel num_threads(2)
#endif 
	//char str[1024];
	//sprintf_s(str, "%s%d%s", "full", count, ".txt");
	//if (!m_ofFullPic.is_open())
	//	m_ofFullPic.open(str);

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
		//m_ofFullPic << 255 - (int)value1 << "  " << 255 - (int)value2 << "  ";
		index += 2;
		//if (index % 1280 == 0)
		//	m_ofFullPic << endl;
		if (index == CELEX5_PIXELS_NUMBER)
		{
			break;
		}
	}
	//count++;
	//m_ofFullPic.close();

	if (m_bIsGeneratingFPN)
	{
		generateFPNimpl();
	}
	else
	{
		createImage();
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
		//if (m_uiRowCount > 800 * 5)
		//{
		//	m_uiRowCount = 0;
		//	//cout << "-------------- m_uiPixelCount = " << m_uiPixelCount << endl;
		//	createImage();
		//	m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
		//}
	}
	createImage();
	m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
}

void CeleX5DataProcessor::parseEventDataFormat1(uint8_t* pData, int dataSize)
{
	//cout << "CeleX5DataProcessor::parseEventDataFormat1: data size = " << vecData.size() << endl;
	if (!m_bLoopModeEnabled)
	{
		if (dataSize != 357001) //358401
		{
			//cout << "CeleX5DataProcessor::parseEventDataFormat1: Not a full package: " << dataSize << endl;
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

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.brightness = adc;
				eventData.t = m_uiEventTCounter;
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
			m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;

			//temperature[11:0] = data_28_1[13:2] = value1[7:0] + value5[5:2]
			temperature = (value1 << 4) + ((0x3C & value5) >> 2);
			m_pCX5ProcessedData->setTemperature(temperature);
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

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.brightness = adc;
				eventData.t = m_uiEventTCounter;
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
			m_emCurrentSensorMode = (CeleX5::CeleX5Mode)op_mode;

			//temperature[11:0] = data[13:2] = value3[7:0] + value7[1:0] + value6[7:6]
			temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 1 (Package_B): mark = " << mark << ", op_mode = " << op_mode << ", temperature = " << temperature << endl;
		}
		if (!m_bLoopModeEnabled)
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
					g_vecEventData.clear();
					g_vecEventData = m_vecEventData;
					m_vecEventData.clear();

					createImage();
					m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
				}
			}
			else if (EventShowByTime == m_emEventShowType) //show by time
			{
				if (m_uiEventTCounter >= m_uiEventTCountForShow)
				{
					m_uiPixelCount = 0;
					m_uiRowCount = 0;
					m_uiEventRowCycleCount = 0;
					m_uiEventTCounter = 0;
					m_uiEventFrameNo++;

					//cout << __FUNCTION__ << ": m_vecEventData.size() = " << m_vecEventData.size() << endl;
					g_vecEventData.clear();
					g_vecEventData = m_vecEventData;
					m_vecEventData.clear();

					createImage();
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
					g_vecEventData.clear();
					g_vecEventData = m_vecEventData;
					m_vecEventData.clear();

					createImage();
					m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
				}
			}
		}
	}
	if (m_bLoopModeEnabled)
	{
		m_uiPixelCount = 0;
		m_uiRowCount = 0;
		m_uiEventRowCycleCount = 0;
		m_uiEventTCounter = 0;
		m_uiEventFrameNo++;

		//cout << __FUNCTION__ << ": m_vecEventData.size() = " << m_vecEventData.size() << endl;
		g_vecEventData.clear();
		g_vecEventData = m_vecEventData;
		m_vecEventData.clear();

		createImage();
		m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
	}
}

void CeleX5DataProcessor::parseEventDataFormat2(uint8_t* pData, int dataSize)
{
	if (!m_bLoopModeEnabled)
	{
		if (dataSize != 357001) //358401
		{
			//cout << "CeleX5DataProcessor::parseEventDataFormat2: Not a full package: " << dataSize << endl;
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
		if (dataID1 == 0x02)
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
		else if (dataID1 == 0x01)
		{
			//col_addr[10:0] = data_14_1[13:3] = value1[7:0] + value5[5:3]
			col = (value1 << 3) + ((0x38 & value5) >> 3);
			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				int index = m_iCurrentRow * CELEX5_COL + col;
				if (index < CELEX5_PIXELS_NUMBER)
					m_pEventCountBuffer[index] += 1;
				m_uiPixelCount++;

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.brightness = 0;
				eventData.t = m_uiEventTCounter;
				m_vecEventData.push_back(eventData);
			}
		}
		else if (dataID1 == 0x00)
		{
			//temperature[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
			temperature = (value1 << 4) + ((0x3C & value5) >> 2);
			m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_A): temperature = " << temperature << endl;
		}
		else if (dataID1 == 0x03)
		{
			//row_time_stamp[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value1 << 4) + ((0x3C & value5) >> 2);
			//cout << "Format 2 (Package_A): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
		}

		//---------- dataID2[1:0] = data[1:0] = value5[7:6] ----------
		uint8_t dataID2 = 0xC0 & value5;
		if (dataID2 == 0x80)
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
		else if (dataID2 == 0x40)
		{
			//col_addr[10:0] = data_14_2[13:3] = value2[7:0] + value6[3:1]
			col = (value2 << 3) + ((0x0E & value6) >> 1);
			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				int index = m_iCurrentRow * CELEX5_COL + col;
				if (index < CELEX5_PIXELS_NUMBER)
					m_pEventCountBuffer[index] += 1;
				m_uiPixelCount++;

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.brightness = 0;
				eventData.t = m_uiEventTCounter;
				m_vecEventData.push_back(eventData);
			}			
		}
		else if (dataID2 == 0x00)
		{
			//temperature[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
			temperature = (value2 << 4) + (0x0F & value6);
			m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_B): temperature = " << temperature << endl;
		}
		else if (dataID2 == 0xC0)
		{
			//row_time_stamp[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value2 << 4) + (0x0F & value6);
			//cout << "Format 2 (Package_B): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
		}

		//---------- dataID3[1:0] = data[1:0] = value6[5:4] ----------
		uint8_t dataID3 = 0x30 & value6;
		if (dataID3 == 0x20)
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
		else if (dataID3 == 0x10)
		{
			//col_addr[10:0] = data_14_3[13:3] = value3[7:0] + value7[1:0] + value6[7]
			col = (value3 << 3) + ((0x03 & value7) << 1) + ((0x80 & value6) >> 7);
			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				int index = m_iCurrentRow * CELEX5_COL + col;
				if (index < CELEX5_PIXELS_NUMBER)
					m_pEventCountBuffer[index] += 1;
				m_uiPixelCount++;

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.brightness = 0;
				eventData.t = m_uiEventTCounter;
				m_vecEventData.push_back(eventData);
			}
		}
		else if (dataID3 == 0x00)
		{
			//temperature[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
			temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_C): temperature = " << temperature << endl;
		}
		else if (dataID3 == 0x30)
		{
			//row_time_stamp[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
			//cout << "Format 2 (Package_C): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
		}

		//---------- dataID4[1:0] = data[1:0] = value7[3:2] ----------
		uint8_t dataID4 = 0x0C & value7;
		if (dataID4 == 0x08)
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
		else if (dataID4 == 0x04)
		{
			//col_addr[10:0] = data_14_4[13:3] = value4[7:0] + value7[7:5]
			col = (value4 << 3) + ((0xE0 & value7) >> 5);		
			if (m_iCurrentRow >= 0 && m_iCurrentRow < CELEX5_ROW)
			{
				int index = m_iCurrentRow * CELEX5_COL + col;
				if (index < CELEX5_PIXELS_NUMBER)
					m_pEventCountBuffer[index] += 1;
				m_uiPixelCount++;

				EventData eventData;
				eventData.row = m_iCurrentRow;
				eventData.col = col;
				eventData.brightness = 0;
				eventData.t = m_uiEventTCounter;
				m_vecEventData.push_back(eventData);
			}
		}
		else if (dataID4 == 0x00)
		{
			//temperature[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
			temperature = (value4 << 4) + ((0xF0 & value7) >> 4);
			m_pCX5ProcessedData->setTemperature(temperature);
			//cout << "Format 2 time full (Package_D): temperature = " << temperature << endl;
		}
		else if (dataID4 == 0x0C)
		{
			//row_time_stamp[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
			m_iLastRowTimeStamp = m_iRowTimeStamp;
			m_iRowTimeStamp = (value4 << 4) + ((0xF0 & value7) >> 4);
			//cout << "Format 2 (Package_D): row_time_stamp = " << m_iRowTimeStamp << endl;

			processMIPIEventTimeStamp();
		}
		if (!m_bLoopModeEnabled)
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
					g_vecEventData.clear();
					g_vecEventData = m_vecEventData;
					m_vecEventData.clear();

					createImage();
					m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
				}
			}
			else if (EventShowByTime == m_emEventShowType) //show by time
			{
				if (m_uiEventTCounter >= m_uiEventTCountForShow)
				{
					m_uiPixelCount = 0;
					m_uiRowCount = 0;
					m_uiEventRowCycleCount = 0;
					m_uiEventTCounter = 0;
					m_uiEventFrameNo++;

					//cout << __FUNCTION__ << ": m_vecEventData.size() = " << m_vecEventData.size() << endl;
					g_vecEventData.clear();
					g_vecEventData = m_vecEventData;
					m_vecEventData.clear();

					createImage();
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
					g_vecEventData.clear();
					g_vecEventData = m_vecEventData;
					m_vecEventData.clear();

					createImage();
					m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);					
				}
			}
		}
	}
	if (m_bLoopModeEnabled)
	{
		m_uiPixelCount = 0;
		m_uiRowCount = 0;
		m_uiEventRowCycleCount = 0;
		m_uiEventTCounter = 0;
		m_uiEventFrameNo++;

		g_vecEventData.clear();
		g_vecEventData = m_vecEventData;
		m_vecEventData.clear();

		createImage();
		m_pCX5Server->notify(CeleX5DataManager::CeleX_Frame_Data);
		//cout << "m_uiEventTCounter = " << m_uiEventTCounter << endl;
	}
#ifdef _ENABLE_LOG_FILE_
	poll_t = clock();
	m_ofLogFile << "  Elapsed Time: " << (poll_t - begin_t) << " (ms)" << std::endl;
#endif
}

CX5SensorDataServer *CeleX5DataProcessor::getSensorDataServer()
{
	return m_pCX5Server;
}

CeleX5ProcessedData *CeleX5DataProcessor::getProcessedData()
{
	return m_pCX5ProcessedData;
}

bool CeleX5DataProcessor::processRowEvent(unsigned char* data)
{
	m_pFPGADataReader->parseRowEvent(data);
	unsigned int t = m_pFPGADataReader->getTFromFPGA();
	//cout << "row = " << m_pFPGADataReader->row() << ",  = " << t << endl;
	//Sleep(200);

	if (!m_bLoopModeEnabled) //fixed mode
	{
		if (m_emCurrentSensorMode == CeleX5::Event_Address_Only_Mode ||
			m_emCurrentSensorMode == CeleX5::Event_Optical_Flow_Mode ||
			m_emCurrentSensorMode == CeleX5::Event_Intensity_Mode)
		{
			bool bUsingRowCount = false;
			if (bUsingRowCount)
			{
				m_uiRowCount++;
				if (m_uiRowCount >= 100 * 5)
				{
					m_uiRowCount = 0;
					return false;
				}
			}
			else
			{
				unsigned int t = m_pFPGADataReader->getTFromFPGA();
				unsigned int tLast = m_pFPGADataReader->getLastTFromFPGA();
				int diffT = t - tLast;
				if (diffT < 0)
				{
					//cout << "t = " << t << ", tLast = " << tLast << endl;
					diffT = diffT + 262144;
				}
				m_uiEventTCounter += diffT;
				if (m_uiEventTCounter > 25000 * m_uiEventFrameTime)
				{
					//cout << "m_uiEventTCounter = " << m_uiEventTCounter << endl;
					m_uiEventTCounter = 0;
					return false;
				}
			}
		}
	}
	return true;
}

bool CeleX5DataProcessor::processColEvent(unsigned char* data)
{
	m_pFPGADataReader->parseColumnEvent(data);
	//
	unsigned int row = m_pFPGADataReader->row();
	unsigned int col = m_pFPGADataReader->column();
	unsigned int adc = m_pFPGADataReader->adc();
	unsigned int t = m_pFPGADataReader->getTFromFPGA();

	//cout << "row = " << row << ", col = " << col << endl;
	//cout << "adc = " << adc << endl;
	if (row < CELEX5_ROW && col < CELEX5_COL)
	{
		// normalize to 0~255
		//adc = normalizeADC(adc);
		adc = (adc >> 4);
		int index = row * CELEX5_COL + col;
		unsigned int type = m_pFPGADataReader->getEventType();
		m_emCurrentSensorMode = CeleX5::CeleX5Mode(type);
		//cout << "type = " << type << endl;
		if (type == CeleX5::Event_Address_Only_Mode) //000
		{
			m_pEventCountBuffer[index] += 1;
		}
		else if (type == CeleX5::Event_Optical_Flow_Mode) //001
		{
			m_pEventADCBuffer[index] = 255 - adc;
		}
		else if (type == CeleX5::Event_Intensity_Mode) //010
		{
			m_pEventADCBuffer[index] = 255 - adc;
			m_pEventCountBuffer[index] += 1;
		}
		else if (type == CeleX5::Full_Picture_Mode ||
			type == CeleX5::Full_Optical_Flow_S_Mode ||
			type == CeleX5::Full_Optical_Flow_M_Mode)
		{
			m_pEventADCBuffer[index] = 255 - adc;
		}
		m_uiPixelCount++;
	}
	return true;
}

bool CeleX5DataProcessor::createImage()
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

	m_pCX5ProcessedData->setSensorMode(m_emCurrentSensorMode);
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
			index = getCurrentIndex(i);
			if (m_pEventCountBuffer[i] > 0)
			{
				value = 255;
				value1 = m_pEventCountBuffer[i] * 30;
			}
			else
			{
				value = value1 = 0;
			}			
			pEventBinaryPic[index] = value;
			pEventCountPic[index] = value1;
			//--- denoised pic ---
			int score = m_pEventCountBuffer[i];
			if (score > 0)
				score = calculateDenoiseScore(m_pEventCountBuffer, i);
			else
				score = 0;
			pEventDenoisedBinaryPic[index] = score;
		}
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Optical_Flow_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);

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
			index = getCurrentIndex(i);
			if (m_pEventADCBuffer[i] > 0)
			{
				value = m_pEventADCBuffer[i] - m_pFpnBuffer[i];
				if (value < 0)
					value = 0;
				if (value > 255)
					value = 255;
				pEventBinaryPic[index] = 255;
				pEventGrayPic[index] = value;
				pEventAccumulatedPic[index] = value; //don't need to clear
				pEventSuperimposedPic[index] = pEventBinaryPic[index] > 0 ? 255 : value;//superimpose
			}
			else
			{
				pEventBinaryPic[index] = 0;
				pEventGrayPic[index] = 0;
				pEventSuperimposedPic[index] = pEventAccumulatedPic[index];
			}
		}
	}
	else if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode)
	{
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; i++)
		{
			index = getCurrentIndex(i);

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
			index = getCurrentIndex(i);

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
		memcpy(pFrameBuffer2, pOpticalFlowPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer3, pOpticalFlowSpeedPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer4, pOpticalFlowDirectionPic, CELEX5_PIXELS_NUMBER);
	}
	else if (m_emCurrentSensorMode == CeleX5::Full_Picture_Mode)
	{
		memcpy(pFrameBuffer1, pFullPic, CELEX5_PIXELS_NUMBER);
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Address_Only_Mode)
	{
		memcpy(pFrameBuffer1, pEventBinaryPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer5, pEventDenoisedBinaryPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer6, pEventCountPic, CELEX5_PIXELS_NUMBER);
	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Intensity_Mode)
	{
		memcpy(pFrameBuffer1, pEventBinaryPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer2, pEventGrayPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer3, pEventAccumulatedPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer4, pEventSuperimposedPic, CELEX5_PIXELS_NUMBER);

	}
	else if (m_emCurrentSensorMode == CeleX5::Event_Optical_Flow_Mode)
	{
		memcpy(pFrameBuffer2, pOpticalFlowPic, CELEX5_PIXELS_NUMBER);
		memcpy(pFrameBuffer1, pEventBinaryPic, CELEX5_PIXELS_NUMBER);
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
			filePath += "/";
#endif 
			if (0 == m_uiISOLevel)
				filePath += "FPN_0.txt";
			else if (1 == m_uiISOLevel)
				filePath += "FPN_1.txt";
			else if (2 == m_uiISOLevel)
				filePath += "FPN_2.txt";
			else if (3 == m_uiISOLevel)
				filePath += "FPN_3.txt";
			else if (4 == m_uiISOLevel)
				filePath += "FPN_4.txt";
			else if (5 == m_uiISOLevel)
				filePath += "FPN_5.txt";
			else if (6 == m_uiISOLevel)
				filePath += "FPN_6.txt";
			else
				filePath += "FPN.txt";
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
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; ++i)
		{
			m_pFpnGenerationBuffer[i] = m_pFpnGenerationBuffer[i] / FPN_CALCULATION_TIMES;
			total += m_pFpnGenerationBuffer[i];
		}
		int avg = total / CELEX5_PIXELS_NUMBER;
		for (int i = 0; i < CELEX5_PIXELS_NUMBER; ++i)
		{
			int d = m_pFpnGenerationBuffer[i] - avg;
			ff << d << "  ";
			if ((i + 1) % 1280 == 0)
				ff << "\n";
		}
		ff.close();
	}
}

bool CeleX5DataProcessor::setFpnFile(const std::string& fpnFile)
{
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

	cout << "fpn count = " << index << endl;

	in.close();
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
}

void CeleX5DataProcessor::setMIPIDataFormat(int format)
{
	m_iMIPIDataFormat = format;
}

void CeleX5DataProcessor::setEventFrameTime(uint32_t value, uint32_t clock) //unit: microsecond
{
	m_uiEventFrameTime = value;
	m_uiEventTCountForShow = value / m_uiEventTUnitList[clock/10 - 1];
	cout << __FUNCTION__ << ": m_uiEventTCountForShow = " << m_uiEventTCountForShow << endl;
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
		return (initIndex / CELEX5_COL * CELEX5_COL + initIndex % CELEX5_COL);
		break;
	}
}

void CeleX5DataProcessor::setRotateType(int type)
{
	m_iRotateType = type;
}
