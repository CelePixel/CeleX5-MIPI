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

#define FPN_PATH    "../Samples/config/FPN_3.txt"

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

int main()
{
	CeleX5 *pCeleX = new CeleX5;
	if (NULL == pCeleX)
		return 0;

	pCeleX->openSensor(CeleX5::CeleX5_MIPI);
	pCeleX->setFpnFile(FPN_PATH);
	pCeleX->setLoopModeEnabled(true);
	pCeleX->setSensorLoopMode(CeleX5::Full_Picture_Mode, 1);
	pCeleX->setSensorLoopMode(CeleX5::Event_Address_Only_Mode, 2);
	pCeleX->setSensorLoopMode(CeleX5::Full_Optical_Flow_S_Mode, 3);

	//SensorDataObserver* pSensorData = new SensorDataObserver(pCeleX->getSensorDataServer());

	int imgSize = 1280 * 800;
	unsigned char* pBuffer1 = new unsigned char[imgSize];
	unsigned char* pBuffer2 = new unsigned char[imgSize];
	unsigned char* pBuffer3 = new unsigned char[imgSize];
	while (true)
	{
		//get fullpic when sensor works in FullPictureMode
		pCeleX->getFullPicBuffer(pBuffer1);
		//full-frame picture
		cv::Mat matFullPic(800, 1280, CV_8UC1, pBuffer1);
		cv::imshow("FullPic", matFullPic);

		pCeleX->getEventPicBuffer(pBuffer2, CeleX5::EventBinaryPic);
		cv::Mat matEventPic(800, 1280, CV_8UC1, pBuffer2);
		cv::imshow("Event Binary Pic", matEventPic);

		pCeleX->getOpticalFlowPicBuffer(pBuffer3, CeleX5::Full_Optical_Flow_Pic);
		//full-frame optical-flow pic
		cv::Mat matOpticalRaw(800, 1280, CV_8UC1, pBuffer3);

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

#ifdef _WIN32
		Sleep(30);
#else
		usleep(1000 * 30);
#endif
	}
	return 1;
}
