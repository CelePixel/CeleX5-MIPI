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

#ifndef DATAPROCESSTHREAD_H
#define DATAPROCESSTHREAD_H

#include "../base/dataqueue.h"
#include "../base/xthread.h"
#include "celex5dataprocessor.h"
#include "../include/celex5/celex5.h"

class DataProcessThread : public XThread
{
public:
	DataProcessThread(const std::string& name);
	~DataProcessThread();

	void addData(uint8_t* data, uint32_t length, std::time_t timeStamp = 0);
	void addData(uint8_t* data, uint32_t length, std::vector<IMURawData> imuData, time_t timeStamp);
	void addData(std::vector<uint8_t> vecData);
	void clearData();
	uint32_t queueSize();
	uint32_t getPackageNo();
	void setPackageNo(uint32_t no);
	void setDeviceType(CeleX5::DeviceType type);
	void setCeleX(CeleX5* pCeleX5);
	void setDataProcessor(CeleX5DataProcessor* pDataProcessor);
	void setIsPlayback(bool state);

	PlaybackState getPlaybackState();
	void setPlaybackState(PlaybackState state);
	void setRecordState(bool bRecord);
	//Whether to display the images when recording
	void setShowImagesEnabled(bool enable);

protected:
	void run() override;

private:
	DataQueue                      m_queueData;
	CeleX5DataProcessor*           m_pDataProcessor;
	std::queue<std::vector<uint8_t>>    m_queueVecData;
	uint32_t                       m_uiPackageNo;
	CeleX5::DeviceType             m_emDeviceType;
	CeleX5*                        m_pCeleX5;
	uint8_t*                       m_pMipiPackage;
	bool                           m_bPlaybackBinFile;
	PlaybackState                  m_emPlaybackState;
	std::vector<IMURawData>			   m_vecIMUData;
	bool                           m_bRecordData;
	bool                           m_bShowImagesEnabled;
};

#endif // DATAPROCESSTHREAD_H
