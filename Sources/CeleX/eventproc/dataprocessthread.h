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

#ifndef DATAPROCESSTHREAD_H
#define DATAPROCESSTHREAD_H

#include "../base/dataqueue.h"
#include "../base/xthread.h"
#include "fpgadataprocessor.h"
#include "../include/celex4/celex4.h"
#include "celex5dataprocessor.h"
#include "../include/celex5/celex5.h"

class DataProcessThread : public XThread
{
public:
    DataProcessThread(const std::string& name = "DataProcessThread");
    ~DataProcessThread();

    void addData(unsigned char* data, long length);
    void clearData();
    void setSensorMode(emSensorMode mode);
    uint32_t queueSize();
    FPGADataProcessor* getDataProcessor();
    PlaybackState getPlaybackState();
    void setPlaybackState(PlaybackState	state);

protected:
    void run() override;

private:
    DataQueue            m_queueData;
    FPGADataProcessor    m_dataProcessor;
    unsigned char*       m_pData;
    //CirDataQueueEx       m_queueDataEx;
    PlaybackState    m_emPlaybackState;
};

class DataProcessThreadEx : public XThread
{
public:
	DataProcessThreadEx(const std::string& name);
	~DataProcessThreadEx();

	void addData(unsigned char* data, long length);
	void addData(vector<uint8_t> vecData);
	void clearData();
	uint32_t queueSize();
	uint32_t packageNo();
	void setPackageNo(uint32_t no);
	CeleX5DataProcessor* getDataProcessor5();
	void setDeviceType(CeleX5::DeviceType type);
	void setCeleX(CeleX5* pCeleX5);
	void setPlayback(bool state);

	PlaybackState getPlaybackState();
	void setPlaybackState(PlaybackState state);

protected:
	void run() override;

private:
	unsigned char*                 m_pData;
	DataQueueEx                    m_queueData;
	CeleX5DataProcessor            m_dataProcessor5;
	std::queue<vector<uint8_t>>    m_queueVecData;
	uint32_t                       m_uiPackageNo;
	CeleX5::DeviceType             m_emDeviceType;
	CeleX5*                        m_pCeleX5;
	vector<uint8_t>                m_vecMIPIPackage;
	bool                           m_bPlaybackBinFile;
	PlaybackState                  m_emPlaybackState;

};

#endif // DATAPROCESSTHREAD_H
