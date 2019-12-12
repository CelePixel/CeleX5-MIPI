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
	pCeleX5->setSensorFixedMode(CeleX5::Optical_Flow_Mode);

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

	uint8_t* pOpticalFlowBuffer = new uint8_t[CELEX5_PIXELS_NUMBER];
	while (true)
	{
		pCeleX5->getOpticalFlowPicBuffer(pOpticalFlowBuffer); //optical-flow raw buffer
		cv::Mat matOpticalRaw(800, 1280, CV_8UC1, pOpticalFlowBuffer);
		//cv::Mat matOpticalRaw = pCeleX5->getOpticalFlowPicMat(CeleX5::Full_Optical_Flow_Pic);	//get optical flow raw data
		cv::imshow("Optical-Flow Buffer - Gray", matOpticalRaw);

		cv::Mat matOpticalSpeed = pCeleX5->getOpticalFlowPicMat(CeleX5::OpticalFlowSpeedPic);	//get optical flow speed buffer
		cv::imshow("Optical-Flow Speed Buffer - Gray", matOpticalSpeed);

		cv::Mat matOpticalDirection = pCeleX5->getOpticalFlowPicMat(CeleX5::OpticalFlowDirectionPic);	//get optical flow direction buffer
		cv::imshow("Optical-Flow Direction Buffer - Gray", matOpticalDirection);

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
		cvWaitKey(30);
	}
	return 1;
}
