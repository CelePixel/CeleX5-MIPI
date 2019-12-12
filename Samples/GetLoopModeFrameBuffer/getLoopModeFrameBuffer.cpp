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

#ifdef _WIN32
#include <windows.h>
#include<process.h>
#else
#include<unistd.h>
#include <signal.h>
#include<pthread.h>
#endif

#define FPN_PATH    "../Samples/config/FPN_2.txt"

CeleX5 *pCeleX5 = new CeleX5;

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
	pCeleX5->setSensorLoopMode(CeleX5::Event_Intensity_Mode, 2);
	pCeleX5->setSensorLoopMode(CeleX5::Optical_Flow_Mode, 3);

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

	uint8_t* pBuffer1 = new uint8_t[CELEX5_PIXELS_NUMBER];
	uint8_t* pBuffer2 = new uint8_t[CELEX5_PIXELS_NUMBER];
	uint8_t* pBuffer3 = new uint8_t[CELEX5_PIXELS_NUMBER];
	while (true)
	{
		//get fullpic when sensor works in FullPictureMode
		pCeleX5->getFullPicBuffer(pBuffer1);
		//full-frame picture
		cv::Mat matFullPic(800, 1280, CV_8UC1, pBuffer1);
		cv::imshow("FullPic", matFullPic);

		pCeleX5->getEventPicBuffer(pBuffer2, CeleX5::EventGrayPic);
		cv::Mat matEventPic(800, 1280, CV_8UC1, pBuffer2);
		cv::imshow("Event Binary Pic", matEventPic);

		pCeleX5->getOpticalFlowPicBuffer(pBuffer3, CeleX5::OpticalFlowPic);
		//full-frame optical-flow pic
		cv::Mat matOpticalRaw(800, 1280, CV_8UC1, pBuffer3);

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

#ifdef _WIN32
		Sleep(30);
#else
		usleep(1000 * 30);
#endif
	}
	return 1;
}
