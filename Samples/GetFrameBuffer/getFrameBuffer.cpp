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

#ifdef _WIN32
#include <windows.h>
#include<process.h>
#else
#include<unistd.h>
#include <signal.h>
#include<pthread.h>
#endif

#define FPN_PATH    "../Samples/config/FPN_3.txt"

CeleX5 *pCeleX5 = new CeleX5;
vector<uint8_t> sensor_buffer;

#ifdef _WIN32
unsigned int _stdcall ReadDataThreadProc(LPVOID lpParameter)
#else
void * ReadDataThreadProc(void *arg)
#endif
{
	cout << "thread function: ReadDataThreadProc!\n";
	while (true)
	{
		if (pCeleX5)
		{
			pCeleX5->getMIPIData(sensor_buffer);
			if (sensor_buffer.size() > 0)
				pCeleX5->parseMIPIData(sensor_buffer.data(), sensor_buffer.size());
			sensor_buffer.clear();
		}
	}
	return 0;
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

	CeleX5::CeleX5Mode sensorMode = CeleX5::Event_Address_Only_Mode;
	pCeleX5->setSensorFixedMode(sensorMode);

	//create read sensor data thread
#ifdef _WIN32
	_beginthreadex(NULL, 0, ReadDataThreadProc, 0, 0, NULL);
#else
	pthread_t t1;
	int a = 1;
	pthread_create(&t1, NULL, ReadDataThreadProc, &a);
#endif

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

	int imgSize = 1280 * 800;
	unsigned char* pBuffer1 = new unsigned char[imgSize];
	while (true)
	{
		if (sensorMode == CeleX5::Full_Picture_Mode)
		{
			//get fullpic when sensor works in FullPictureMode
			pCeleX5->getFullPicBuffer(pBuffer1); //full pic
			cv::Mat matFullPic(800, 1280, CV_8UC1, pBuffer1);
			cv::imshow("FullPic", matFullPic);
			cvWaitKey(10);
		}
		else if (sensorMode == CeleX5::Event_Address_Only_Mode)
		{
			//get buffers when sensor works in EventMode
			pCeleX5->getEventPicBuffer(pBuffer1, CeleX5::EventBinaryPic); //event binary pic
			cv::Mat matEventPic(800, 1280, CV_8UC1, pBuffer1);
			cv::imshow("Event-EventBinaryPic", matEventPic);
			cvWaitKey(30);
		}
	}
	return 1;
}