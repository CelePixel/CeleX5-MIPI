/*
* Copyright (c) 2017-2018 CelePixel Technology Co. Ltd. All Rights Reserved
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

#define FPN_PATH    "../Samples/config/FPN_Gain2.txt"

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
		m_pServer->registerData(this, CeleX5DataManager::CeleX_Frame_Data);
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
		//get fullpic when sensor works in FullPictureMode
		if (pSensorData->getFullPicBuffer())
		{
			//full-frame picture
			cv::Mat matFullPic(800, 1280, CV_8UC1, pSensorData->getFullPicBuffer()); 
			cv::imshow("FullPic", matFullPic);
			cv::waitKey(1);
		}
	}
	else if (CeleX5::Event_Address_Only_Mode == sensorMode)
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
	else if (CeleX5::Full_Optical_Flow_S_Mode == sensorMode)
	{
		//get buffers when sensor works in FullPic_Event_Mode
		if (pSensorData->getOpticalFlowPicBuffer(CeleX5::Full_Optical_Flow_Pic))
		{
			//full-frame optical-flow pic
			cv::Mat matOpticalRaw(800, 1280, CV_8UC1, pSensorData->getOpticalFlowPicBuffer(CeleX5::Full_Optical_Flow_Pic));
	
			//optical-flow raw data - display color image
			cv::Mat matOpticalColor(800, 1280, CV_8UC3);
			uchar* pRaw = matOpticalColor.ptr<uchar>(0);
			int index = 0;
			for (int i = 0; i < matOpticalColor.rows; ++i)
			{
				cv::Vec3b *p = matOpticalColor.ptr<cv::Vec3b>(i);
				for (int j = 0; j < matOpticalColor.cols; ++j)
				{
					int value = matOpticalRaw.at<uchar>(i, j);
					//std::cout << value << std::endl;
					if (value == 0)
					{
						p[j][0] = 0;
						p[j][1] = 0;
						p[j][2] = 0;
					}
					else if (value < 50)	//blue
					{
						p[j][0] = 255;
						p[j][1] = 0;
						p[j][2] = 0;
					}
					else if (value < 100)
					{
						p[j][0] = 255;
						p[j][1] = 255;
						p[j][2] = 0;
					}
					else if (value < 150)	//green
					{
						p[j][0] = 0;
						p[j][1] = 255;
						p[j][2] = 0;
					}
					else if (value < 200)
					{
						p[j][0] = 0;
						p[j][1] = 255;
						p[j][2] = 255;
					}
					else	//red
					{
						p[j][0] = 0;
						p[j][1] = 0;
						p[j][2] = 255;
					}
				}
			}
			cv::imshow("Optical-Flow Buffer - Color", matOpticalColor);
			cvWaitKey(1);
		}
	}
}

int main()
{
	CeleX5 *pCeleX = new CeleX5;
	if (NULL == pCeleX)
		return 0;
	
	pCeleX->openSensor(CeleX5::CeleX5_MIPI);
	pCeleX->setFpnFile(FPN_PATH);
	pCeleX->setLoopModeEnabled(true);
	pCeleX->setSensorLoopMode(CeleX5::Full_Picture_Mode, 1);
	pCeleX->setSensorLoopMode(CeleX5::Event_Intensity_Mode, 2);
	pCeleX->setSensorLoopMode(CeleX5::Full_Optical_Flow_S_Mode, 3);

	SensorDataObserver* pSensorData = new SensorDataObserver(pCeleX->getSensorDataServer());

	while (true)
	{
#ifdef _WIN32
		Sleep(5);
#else
		usleep(1000 * 5);
#endif
	}
	return 1;
}
