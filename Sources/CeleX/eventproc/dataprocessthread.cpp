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
#include "dataprocessthread.h"

DataProcessThread::DataProcessThread(const std::string &name)
	: XThread(name)
	, m_uiPackageNo(0)
	, m_emDeviceType(CeleX5::Unknown_Devive)
	, m_bPlaybackBinFile(false)
	, m_emPlaybackState(NoBinPlaying)
	, m_bRecordData(false)
	, m_bShowImagesEnabled(true)
{
	m_pMipiPackage = new uint8_t[1536001];
}

DataProcessThread::~DataProcessThread()
{
	delete[] m_pMipiPackage;
	m_pMipiPackage = nullptr;
}

void DataProcessThread::addData(uint8_t* data, uint32_t length, time_t timeStamp)
{
	m_queueData.push(data, length, timeStamp);
}

void DataProcessThread::addData(uint8_t* data, uint32_t length, std::vector<IMURawData> imuData, std::time_t timeStamp)
{
	m_queueData.push(data, length, imuData, timeStamp);
}

void DataProcessThread::addData(std::vector<uint8_t> vecData)
{
	//cout << "size = " << vecData.size() << endl;
	if (vecData.size() > 0)
	{
		std::vector<uint8_t> vec;
		vec.swap(vecData);
		//cout << "DataProcessThread::addData = " << vec.size() << endl;
		m_queueVecData.push(vec);
	}
	//cout << m_queueVecData.size() << endl;
}

void DataProcessThread::clearData()
{
	m_queueData.clear();
	m_uiPackageNo = 0;
}

uint32_t DataProcessThread::queueSize()
{
	return m_queueData.size();
}

uint32_t DataProcessThread::getPackageNo()
{
	return m_uiPackageNo;
}

void DataProcessThread::setPackageNo(uint32_t no)
{
	m_uiPackageNo = no;
}

void DataProcessThread::setDeviceType(CeleX5::DeviceType type)
{
	m_emDeviceType = type;
}

void DataProcessThread::setCeleX(CeleX5* pCeleX5)
{
	m_pCeleX5 = pCeleX5;
}

void DataProcessThread::setDataProcessor(CeleX5DataProcessor* pDataProcessor)
{
	m_pDataProcessor = pDataProcessor;
}

void DataProcessThread::setIsPlayback(bool state)
{
	m_bPlaybackBinFile = state;
}

PlaybackState DataProcessThread::getPlaybackState()
{
	return m_emPlaybackState;
}

void DataProcessThread::setPlaybackState(PlaybackState state)
{
	m_emPlaybackState = state;
}

void DataProcessThread::setRecordState(bool bRecord)
{
	m_bRecordData = bRecord;
}

void DataProcessThread::setShowImagesEnabled(bool enable)
{
	m_bShowImagesEnabled = enable;
}

void DataProcessThread::run()
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
		if (m_bPlaybackBinFile) //for playback
		{
			if (m_queueData.size() > 0)
			{
				uint32_t dataLen = 0;
				time_t timestamp = 0;
				m_queueData.pop(m_pMipiPackage, &dataLen, m_vecIMUData, &timestamp);
				//std::cout << "------------------" << "pop data size = " << dataLen << std::endl;
				if (dataLen > 0)
				{
					m_pDataProcessor->processMIPIData(m_pMipiPackage, dataLen, timestamp, m_vecIMUData);
					//cout << __FUNCTION__ << ": imu size = " << m_vecIMUData.size() << endl;
					if (m_vecIMUData.size() > 0)
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
		else //--- for real display ---
		{
			std::time_t time_stamp_end = 0;
			std::vector<IMURawData> imu_data;
			uint32_t dataLen = 0;
			m_pCeleX5->getCeleXRawData(m_pMipiPackage, dataLen, time_stamp_end, imu_data);
			if (dataLen > 0)
			{
				//std::cout << "dataLen = " << dataLen << std::endl;
				if (!m_bRecordData || (m_bRecordData && m_bShowImagesEnabled))
					m_pDataProcessor->processMIPIData(m_pMipiPackage, dataLen, time_stamp_end, imu_data);
			}
		}
	}
}
