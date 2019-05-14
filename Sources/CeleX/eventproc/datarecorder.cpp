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

DataRecorder::DataRecorder()
	: m_bRecording(false)
	, m_iTimeStampStart(0)
	, m_uiPackageCount(0)
{

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
	XBase base;
	m_iTimeStampStart = base.getTimeStamp();

	m_ofstreamRecord.open(filePath.c_str(), std::ios::binary);
	if (!m_ofstreamRecord.is_open())
		cout << "Can't open recording file." << endl;

	CeleX5::BinFileAttributes header;
	m_ofstreamRecord.write((char*)&header, sizeof(CeleX5::BinFileAttributes));

	m_bRecording = true;
	m_uiPackageCount = 0;
}

void DataRecorder::stopRecording(uint32_t clock, int mode)
{
	XBase base;
	int iTimeRecorded = base.getTimeStamp() - m_iTimeStampStart;

	int hour = iTimeRecorded / 3600;
	int minute = (iTimeRecorded % 3600) / 60;
	int second = (iTimeRecorded % 3600) % 60;

	// write a header
	char header[8];
	header[0] = second;
	header[1] = minute;
	header[2] = hour;
	header[3] = 0;
	header[4] = 0;
	header[5] = 0;
	header[6] = clock;
	header[7] = mode;

	m_ofstreamRecord.seekp(0, ios::beg);
	m_ofstreamRecord.write(header, sizeof(header));
	m_bRecording = false;
	m_ofstreamRecord.close();
}

void DataRecorder::stopRecording(CeleX5::BinFileAttributes* header)
{
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
	m_bRecording = false;
	m_ofstreamRecord.close();
}

void DataRecorder::writeData(unsigned char* pData, long length)
{
	m_ofstreamRecord.write((char*)pData, length);
}

bool DataRecorder::writeData(vector<uint8_t> vecData)
{
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
	if (!writeData(vecData)) //write the data buffer
	{
		return false;
	}
	m_ofstreamRecord.write((char*)&time_stamp_end, 8);	//timestamp of frame
	// write the imu data
	uint32_t size = imuData.size();
	m_ofstreamRecord.write((char*)&size, 4);
	m_ofstreamRecord.write((char*)imuData.data(), imuData.size()*sizeof(IMURawData));
	return true;
}