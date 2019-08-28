/*
* Copyright (c) 2017-2017-2018 CelePixel Technology Co. Ltd. All Rights Reserved
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

#include <opencv2/opencv.hpp>
#include <celex5/celex5.h>
#include <celex5/celex5datamanager.h>
#include <celex5/celex5processeddata.h>

#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#endif

#define FPN_PATH    "../Samples/config/FPN_3.txt"
#define BIN_FILE    "YOUR_BIN_FILE_PATH.bin"	//your bin file path

CeleX5 *pCeleX5 = new CeleX5;

class SensorDataObserver : public CeleX5DataManager
{
public:
	SensorDataObserver(CX5SensorDataServer* pServer)
	{
		m_pServer = pServer;
		m_pServer->registerData(this, CeleX5DataManager::CeleX_Frame_Data);
	}
	~SensorDataObserver()
	{
		m_pServer->unregisterData(this, CeleX5DataManager::CeleX_Frame_Data);
	}
	virtual void onFrameDataUpdated(CeleX5ProcessedData* pSensorData);//overrides Observer operation

	CX5SensorDataServer* m_pServer;
};

void SensorDataObserver::onFrameDataUpdated(CeleX5ProcessedData* pSensorData)
{
	if (NULL == pSensorData)
		return;
	CeleX5::CeleX5Mode sensorMode = pSensorData->getSensorMode();
	if (CeleX5::Full_Picture_Mode == sensorMode)
	{
		//full-frame picture
		cv::Mat matFullPic(800, 1280, CV_8UC1, pSensorData->getFullPicBuffer());
		cv::imshow("FullPic", matFullPic);
		cv::waitKey(1);
	}
	else if (CeleX5::Event_Off_Pixel_Timestamp_Mode == sensorMode)
	{
		//get buffers when sensor works in EventMode
		if (pSensorData->getEventPicBuffer(CeleX5::EventBinaryPic))
		{
			//event binary pic
			cv::Mat matEventPic(800, 1280, CV_8UC1, pSensorData->getEventPicBuffer(CeleX5::EventBinaryPic));
			cv::imshow("Event Binary Pic", matEventPic);
			cvWaitKey(1);
		}
	}
	else if (CeleX5::Optical_Flow_Mode == sensorMode)
	{
		//full-frame optical-flow pic
		cv::Mat matOpticalFlow(800, 1280, CV_8UC1, pSensorData->getOpticalFlowPicBuffer(CeleX5::Full_Optical_Flow_Pic));
		cv::imshow("Optical-Flow Pic", matOpticalFlow);
		cvWaitKey(1);
	}
}

int main()
{
	if (pCeleX5 == NULL)
		return 0;
	pCeleX5->openSensor(CeleX5::CeleX5_MIPI);
	bool success = pCeleX5->openBinFile(BIN_FILE);	//open the bin file
	pCeleX5->setFpnFile(FPN_PATH);
	CeleX5::CeleX5Mode sensorMode = (CeleX5::CeleX5Mode)pCeleX5->getBinFileAttributes().loopA_mode;

	SensorDataObserver* pSensorData = new SensorDataObserver(pCeleX5->getSensorDataServer());

	while (true)
	{
		if (pCeleX5)
		{
			pCeleX5->readBinFileData();	//start reading the bin file
		}
#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
	return 1;
}
