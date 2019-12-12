/*
* Copyright (c) 2017-2020 CelePixel Technology Co. Ltd. All Rights Reserved
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

#define MAT_ROWS 800
#define MAT_COLS 1280
#define FPN_PATH    "../Samples/config/FPN_2.txt"

#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#include <signal.h>
#endif

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

uint8_t* pImageBuffer = new uint8_t[CELEX5_PIXELS_NUMBER];
void SensorDataObserver::onFrameDataUpdated(CeleX5ProcessedData* pSensorData)
{
	if (nullptr == pSensorData)
		return;
	CeleX5::CeleX5Mode sensorMode = pSensorData->getSensorMode();
	if (CeleX5::Full_Picture_Mode == sensorMode)
	{
		time_t time_stamp = 0;
		pCeleX5->getFullPicBuffer(pImageBuffer, time_stamp);
		//cout << "----------- F time_stamp = " << time_stamp << endl;

		//get fullpic when sensor works in FullPictureMode
		cv::Mat matFullPic(800, 1280, CV_8UC1, pImageBuffer);
		cv::imshow("FullPic", matFullPic);
		cv::waitKey(1);
	}
	else if (CeleX5::Event_Off_Pixel_Timestamp_Mode == sensorMode)
	{
		//get buffers when sensor works in EventMode
		std::time_t timestamp = 0;
		pCeleX5->getEventPicBuffer(pImageBuffer, timestamp, CeleX5::EventBinaryPic);

		//event binary pic
		cv::Mat matEventPic(800, 1280, CV_8UC1, pImageBuffer);
		cv::imshow("Event Binary Pic", matEventPic);
		cvWaitKey(1);
	}
	else if (CeleX5::Event_In_Pixel_Timestamp_Mode == sensorMode)
	{
		std::vector<EventData> vecEvent;
		pCeleX5->getEventDataVector(vecEvent);
		int dataSize = vecEvent.size();
		//cout << "data size = " << dataSize << endl;
		cv::Mat mat = cv::Mat::zeros(cv::Size(1280, 800), CV_8UC1);
		for (int i = 0; i < dataSize; i++)
		{
			mat.at<uchar>(800 - vecEvent[i].row - 1, 1280 - vecEvent[i].col - 1) = 255;
			//cout << vecEvent[i].row << ", " << vecEvent[i].col << endl;
		}
		if (dataSize > 0)
		{
			cv::imshow("show", mat);
			cv::waitKey(1);
		}
	}
	else if (CeleX5::Optical_Flow_Mode == sensorMode)
	{
		//get buffers when sensor works in FullPic_Event_Mode
		pCeleX5->getOpticalFlowPicBuffer(pImageBuffer, CeleX5::OpticalFlowPic);
		//full-frame optical-flow pic
		cv::Mat matOpticalRaw(800, 1280, CV_8UC1, pImageBuffer);

		//optical-flow raw data - display color image
		cv::Mat matOpticalColor(800, 1280, CV_8UC3);

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

#ifdef _WIN32
bool exit_handler(DWORD fdwctrltype)
{
	switch (fdwctrltype)
	{
		//ctrl-close: confirm that the user wants to exit.
	case CTRL_CLOSE_EVENT:
	case CTRL_C_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		delete pCeleX5;
		pCeleX5 = NULL;
		return(true);
	default:
		return false;
	}
}
#else
void exit_handler(int sig_num)
{
	printf("SIGNAL received: num =%d\n", sig_num);
	if (sig_num == 1 || sig_num == 2 || sig_num == 3 || sig_num == 9 || sig_num == 15)
	{
		delete pCeleX5;
		pCeleX5 = NULL;
		exit(0);
	}
}
#endif

int main()
{
	if (NULL == pCeleX5)
		return 0;
	
	pCeleX5->openSensor(CeleX5::CeleX5_MIPI);
	pCeleX5->setFpnFile(FPN_PATH);
	pCeleX5->setLoopModeEnabled(true);
	pCeleX5->setSensorLoopMode(CeleX5::Full_Picture_Mode, 1);
	pCeleX5->setSensorLoopMode(CeleX5::Event_In_Pixel_Timestamp_Mode, 2);
	pCeleX5->setSensorLoopMode(CeleX5::Optical_Flow_Mode, 3);

	SensorDataObserver* pSensorData = new SensorDataObserver(pCeleX5->getSensorDataServer());

#ifdef _WIN32
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)exit_handler, true);
#else
	// install signal use sigaction
	struct sigaction sig_action;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = 0;
	sig_action.sa_handler = exit_handler;
	sigaction(SIGHUP, &sig_action, NULL);  // 1
	sigaction(SIGINT, &sig_action, NULL);  // 2
	sigaction(SIGQUIT, &sig_action, NULL); // 3
	sigaction(SIGKILL, &sig_action, NULL); // 9
	sigaction(SIGTERM, &sig_action, NULL); // 15
#endif

	while (true)
	{
#ifdef _WIN32
		Sleep(1);
#else
		usleep(1000);
#endif
	}
	return 1;
}
