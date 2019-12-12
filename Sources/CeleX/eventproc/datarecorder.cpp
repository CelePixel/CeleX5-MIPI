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

#include "datarecorder.h"

#ifdef _WIN32
#include <Windows.h>
CRITICAL_SECTION    g_csDataRecorder;
#else
#endif

#define TIMESTAMP_LENGTH       8
#define PACKAGE_SIZE_LENGTH    4
#define IMU_SIZE_LENGTH        4
#define MAX_PACKAGE_SIZE       1536001  

DataRecorder::DataRecorder()
	: m_bRecording(false)
	, m_iTimeStampStart(0)
	, m_uiPackageCount(0)
{
#ifdef _WIN32
	InitializeCriticalSection(&g_csDataRecorder);
#else
#endif
}

DataRecorder::~DataRecorder()
{
}

/*
*  @function:  startRecording
*  @brief   :  start recording raw data of CeleX-5 sensors
*  @input   :  filePath: bin file path and name to record
*  @output  :
*  @return  : true is successful to start recording
*/
bool DataRecorder::startRecording(std::string filePath)
{
	//cout << __FUNCTION__ << ": start open file!" << endl;
	m_iTimeStampStart = getLocalTimestamp();

	m_ofstreamRecord.open(filePath.c_str(), std::ios::binary);
	if (!m_ofstreamRecord.is_open())
	{
		std::cout << __FUNCTION__ << "Can't open recording file." << std::endl;
		return false;
	}
	std::cout << __FUNCTION__ << ": filePath = " << filePath << std::endl;

	CeleX5::BinFileAttributes header;
	m_ofstreamRecord.write((char*)&header, sizeof(CeleX5::BinFileAttributes));

	m_bRecording = true;
	m_uiPackageCount = 0;

	return true;
}

/*
*  @function:  stopRecording
*  @brief   :  stop recording raw data of CeleX-5 sensors
*  @input   :  header: the header of bin file
*  @output  :
*  @return  :
*/
void DataRecorder::stopRecording(CeleX5::BinFileAttributes* header)
{
	//cout << __FUNCTION__ << ": close file!" << endl;
#ifdef _WIN32
	EnterCriticalSection(&g_csDataRecorder);
#endif

	m_bRecording = false;

	int iTimeRecorded = getLocalTimestamp() - m_iTimeStampStart;

	int hour = iTimeRecorded / 3600;
	int minute = (iTimeRecorded % 3600) / 60;
	int second = (iTimeRecorded % 3600) % 60;

	// write a header
	header->hour = hour;
	header->minute = minute;
	header->second = second;
	header->packageCount = m_uiPackageCount;

	m_ofstreamRecord.seekp(0, std::ios::beg);
	m_ofstreamRecord.write((char*)header, sizeof(CeleX5::BinFileAttributes));
	m_ofstreamRecord.flush();
	m_ofstreamRecord.close();

#ifdef _WIN32
	LeaveCriticalSection(&g_csDataRecorder);
#endif
}

/*
*  @function: writeData
*  @brief   : wirte vecData (sensor data) into bin file
*  @input   : vecData: data vector to be written
*  @output  :
*  @return  : true
*/
bool DataRecorder::writeData(uint8_t* pData, uint32_t length)
{
	if (!m_ofstreamRecord.is_open())
		return false;
	uint32_t size = length;
	if (length > MAX_PACKAGE_SIZE)
		return false;

	m_uiPackageCount++;

	//--- write package size ---
	m_ofstreamRecord.write((char*)&size, PACKAGE_SIZE_LENGTH);

	//--- write package data ---
	m_ofstreamRecord.write((char*)pData, size);

	return true;
}

/*
*  @function: writeData
*  @brief   : wirte vecData (sensor data), sensor data timestamp and imuData into bin file
*  @input   : vecData: data vector to be written       
*             time_stamp_end: the pc timestamp when received the vecData (sensor data)
*             imuData: a vector of imu raw data 
*  @output  :
*  @return  :  rue is successful to write data
*/
bool DataRecorder::writeData(uint8_t* pData, uint32_t length, std::time_t time_stamp_end, std::vector<IMURawData> imuData)
{
#ifdef _WIN32
	EnterCriticalSection(&g_csDataRecorder);
#endif

	if (!m_ofstreamRecord.is_open())
	{
#ifdef _WIN32
		LeaveCriticalSection(&g_csDataRecorder);
#endif
		return false;
	}		

	if (!writeData(pData, length)) //write the data buffer
	{
#ifdef _WIN32
		LeaveCriticalSection(&g_csDataRecorder);
#endif
		return false;
	}
	m_ofstreamRecord.write((char*)&time_stamp_end, TIMESTAMP_LENGTH);	//timestamp of frame
	// write the imu data
	uint32_t size = imuData.size();
	m_ofstreamRecord.write((char*)&size, IMU_SIZE_LENGTH);
	m_ofstreamRecord.write((char*)imuData.data(), imuData.size()*sizeof(IMURawData));

#ifdef _WIN32
	LeaveCriticalSection(&g_csDataRecorder);
#endif

	return true;
}

/*
*  @function: getLocalTime
*  @brief   : get local time and convert it into an integer
*  @input   :
*  @output  :
*  @return  : an integer type of local time
*/
int DataRecorder::getLocalTimestamp()
{
#ifdef _WIN32
	time_t ttCurrentT = time(NULL);
	tm stTm;
	localtime_s(&stTm, &ttCurrentT);

	return 3600 * stTm.tm_hour + 60 * stTm.tm_min + stTm.tm_sec;
#else
	time_t ttCurrentT = time(NULL);
	struct tm* pTm;
	pTm = localtime(&ttCurrentT);
	return 3600 * pTm->tm_hour + 60 * pTm->tm_min + pTm->tm_sec;
#endif
}