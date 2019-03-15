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

#include "dataprocessthread.h"
#include <iostream>

using namespace std;

DataProcessThreadEx::DataProcessThreadEx(const std::string &name)
	: XThread(name)
	, m_uiPackageNo(0)
	, m_emDeviceType(CeleX5::Unknown_Devive)
	, m_bPlaybackBinFile(false)
	, m_emPlaybackState(NoBinPlaying)
	, m_bRecordData(false)
	, m_bShowImagesEnabled(true)
{
	m_pData = new unsigned char[1536001];
}

DataProcessThreadEx::~DataProcessThreadEx()
{
	delete[] m_pData;
	m_pData = NULL;
}

void DataProcessThreadEx::addData(unsigned char *data, long length, time_t timeStamp)
{
	m_queueData.push(data, length, timeStamp);
}

void DataProcessThreadEx::addIMUData(vector<IMURawData> imuData)
{
	//cout << imuData.size() << endl;
	m_vecIMUData.swap(imuData);
}

void DataProcessThreadEx::addData(vector<uint8_t> vecData)
{
	//cout << "size = " << vecData.size() << endl;
	if (vecData.size() > 0)
	{
		vector<uint8_t> vec;
		vec.swap(vecData);
		//cout << "DataProcessThread::addData = " << vec.size() << endl;
		m_queueVecData.push(vec);
	}
	//cout << m_queueVecData.size() << endl;
}

void DataProcessThreadEx::clearData()
{
	m_queueData.clear();
	m_uiPackageNo = 0;
}

uint32_t DataProcessThreadEx::queueSize()
{
	return m_queueData.size();
}

uint32_t DataProcessThreadEx::getPackageNo()
{
	return m_uiPackageNo;
}

void DataProcessThreadEx::setPackageNo(uint32_t no)
{
	m_uiPackageNo = no;
}

CeleX5DataProcessor *DataProcessThreadEx::getDataProcessor5()
{
	return &m_dataProcessor5;
}

void DataProcessThreadEx::setDeviceType(CeleX5::DeviceType type)
{
	m_emDeviceType = type;
}

void DataProcessThreadEx::setCeleX(CeleX5* pCeleX5)
{
	m_pCeleX5 = pCeleX5;
}

void DataProcessThreadEx::setIsPlayback(bool state)
{
	m_bPlaybackBinFile = state;
}

PlaybackState DataProcessThreadEx::getPlaybackState()
{
	return m_emPlaybackState;
}

void DataProcessThreadEx::setPlaybackState(PlaybackState state)
{
	m_emPlaybackState = state;
}

void DataProcessThreadEx::setRecordState(bool bRecord)
{
	m_bRecordData = bRecord;
}

void DataProcessThreadEx::setShowImagesEnabled(bool enable)
{
	m_bShowImagesEnabled = enable;
}

void DataProcessThreadEx::run()
{
	while (m_bRun)
	{
		//cout << "---------- DataProcessThread::run ----------" << endl;
#ifndef _WIN32
		pthread_mutex_lock(&m_mutex);
		while (m_bSuspended)
		{
			pthread_cond_wait(&m_cond, &m_mutex);
		}
		pthread_mutex_unlock(&m_mutex);
#endif
		if (CeleX5::CeleX5_OpalKelly == m_emDeviceType)
		{
			if (m_queueData.size() > 0)
			{
				unsigned char* data = 0;
				long dataLen = 0;
				time_t timestamp = 0;
				m_queueData.pop(data, &dataLen, &timestamp);
				if (dataLen > 0)
				{
					m_dataProcessor5.processData(data, dataLen);
					delete[] data;
				}
				//cout << "dataLen = " << dataLen << endl;
			}
		}
		else if (CeleX5::CeleX5_MIPI == m_emDeviceType)
		{
			if (m_bPlaybackBinFile)
			{
				//for playback
				if (m_queueData.size() > 0)
				{
					long dataLen = 0;
					time_t timestamp = 0;
					m_queueData.pop(m_pData, &dataLen, &timestamp);
					//cout << "------------------" << "pop data size = " << dataLen << endl;
					if (dataLen > 0)
					{
						m_dataProcessor5.processMIPIData(m_pData, dataLen, timestamp, m_vecIMUData);
						if(m_vecIMUData.size()>0)
							m_vecIMUData.clear();
						m_uiPackageNo++;
					}
				}
				else
				{
					if (m_emPlaybackState == BinReadFinished)
					{
						m_emPlaybackState = PlayFinished;
					}
				}
			}
			else
			{
				//--- for real display ---
				//m_pCeleX5->getMIPIData(m_vecMIPIPackage);
				std::time_t time_stamp_end = 0;
				vector<IMURawData> imu_data;
				m_pCeleX5->getMIPIData(m_vecMIPIPackage, time_stamp_end, imu_data);
				if (m_vecMIPIPackage.size() > 0)
				{
					if (!m_bRecordData || (m_bRecordData && m_bShowImagesEnabled))
						m_dataProcessor5.processMIPIData(m_vecMIPIPackage.data(), m_vecMIPIPackage.size(), time_stamp_end, imu_data);
					m_vecMIPIPackage.clear();
				}
			}
		}
	}
}
