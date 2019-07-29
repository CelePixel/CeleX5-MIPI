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

#include "datarecorder.h"
#include "../base/xbase.h"

#ifdef _WIN32
#include <Windows.h>
CRITICAL_SECTION    g_csDataRecorder;
#else
#endif

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

bool DataRecorder::isRecording()
{
	return m_bRecording;
}

void DataRecorder::startRecording(std::string filePath)
{
	//cout << __FUNCTION__ << ": start open file!" << endl;
	XBase base;
	m_iTimeStampStart = base.getTimeStamp();

	m_ofstreamRecord.open(filePath.c_str(), std::ios::binary);
	if (!m_ofstreamRecord.is_open())
	{
		cout << "Can't open recording file." << endl;
		return;
	}
	cout << __FUNCTION__ << ": filePath = " << filePath << endl;

	CeleX5::BinFileAttributes header;
	m_ofstreamRecord.write((char*)&header, sizeof(CeleX5::BinFileAttributes));

	m_bRecording = true;
	m_uiPackageCount = 0;
}

void DataRecorder::stopRecording(CeleX5::BinFileAttributes* header)
{
	//cout << __FUNCTION__ << ": close file!" << endl;
#ifdef _WIN32
	EnterCriticalSection(&g_csDataRecorder);
#endif

	m_bRecording = false;

	XBase base;
	int iTimeRecorded = base.getTimeStamp() - m_iTimeStampStart;

	int hour = iTimeRecorded / 3600;
	int minute = (iTimeRecorded % 3600) / 60;
	int second = (iTimeRecorded % 3600) % 60;

	//cout << iTimeRecorded << endl;

	// write a header
	header->hour = hour;
	header->minute = minute;
	header->second = second;
	header->package_count = m_uiPackageCount;

	m_ofstreamRecord.seekp(0, ios::beg);
	m_ofstreamRecord.write((char*)header, sizeof(CeleX5::BinFileAttributes));
	m_ofstreamRecord.flush();
	m_ofstreamRecord.close();

#ifdef _WIN32
	LeaveCriticalSection(&g_csDataRecorder);
#endif
}

void DataRecorder::writeData(unsigned char* pData, long length)
{
	m_ofstreamRecord.write((char*)pData, length);
}

bool DataRecorder::writeData(vector<uint8_t> vecData)
{
	if (!m_ofstreamRecord.is_open())
		return false;
	uint32_t size = vecData.size();
	if (size > 1536001)
		return false;

	m_uiPackageCount++;

	//--- write package size ---
	m_ofstreamRecord.write((char*)&size, 4);

	//--- write package data ---
	/*for (int i = 0; i < vecData.size(); i++)
	{
		*(pData+i) = vecData.at(i);
	}*/
	m_ofstreamRecord.write((char*)vecData.data(), size);
	return true;
}

bool DataRecorder::writeData(vector<uint8_t> vecData, time_t time_stamp_end, vector<IMURawData> imuData)
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

	if (!writeData(vecData)) //write the data buffer
	{
#ifdef _WIN32
		LeaveCriticalSection(&g_csDataRecorder);
#endif
		return false;
	}
	m_ofstreamRecord.write((char*)&time_stamp_end, 8);	//timestamp of frame
	// write the imu data
	uint32_t size = imuData.size();
	m_ofstreamRecord.write((char*)&size, 4);
	m_ofstreamRecord.write((char*)imuData.data(), imuData.size()*sizeof(IMURawData));

#ifdef _WIN32
	LeaveCriticalSection(&g_csDataRecorder);
#endif

	return true;
}
