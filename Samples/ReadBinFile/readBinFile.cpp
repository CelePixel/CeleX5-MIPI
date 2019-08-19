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
#define BIN_FILE    "E:/API/CeleX5/MIPI/Released_API/CeleX5_MP_V1.5/Samples/test.bin"	//your bin file path

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

int i = 1000;
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

		//save fullpic jpg
		if (!pCeleX5->getFullPicMat().empty())
		{
			cv::imwrite("D:/Datasets/" + std::to_string(i) + "_FullPic.jpg", matFullPic);
		}
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

		//save fullpic jpg
		if (!pCeleX5->getFullPicMat().empty())
		{
			cv::imwrite("D:/Datasets/" + std::to_string(i) + "_OpticalFlow.jpg", matOpticalFlow);
		}
		i++;
	}
}

std::ifstream ffff;
int main()
{
//	if (pCeleX5 == NULL)
//		return 0;
//	pCeleX5->openSensor(CeleX5::CeleX5_MIPI);
//	bool success = pCeleX5->openBinFile(BIN_FILE);	//open the bin file
//	pCeleX5->setFpnFile(FPN_PATH);
//	CeleX5::CeleX5Mode sensorMode = (CeleX5::CeleX5Mode)pCeleX5->getBinFileAttributes().loopA_mode;
//
//	SensorDataObserver* pSensorData = new SensorDataObserver(pCeleX5->getSensorDataServer());
//
//	while (true)
//	{
//		if (pCeleX5)
//		{
//			pCeleX5->readBinFileData();	//start reading the bin file
//		}
//#ifdef _WIN32
//		Sleep(1);
//#else
//		usleep(1000);
//#endif
//	}
//	return 1;

	ffff.open(BIN_FILE, std::ios::binary);
	char* pData = new char[357000];
	unsigned char* m_pEventCountBuffer = new unsigned char[1024000];
	memset(m_pEventCountBuffer, 0, 1024000);
	int event_count = 0;
	while (ffff.is_open())
	{
		cout << "----------------------- OK" << endl;

		cout << "len = " << ffff.tellg() << endl;
		ffff.read((char*)pData, 357000);

		int index = 0;
		int row = 0;
		int col = 0;
		int adc = 0;
		int temperature = 0;
		for (int i = 0; i < 357000 - 6; i += 7)
		{
			uint8_t value1 = *(pData + i);
			uint8_t value2 = *(pData + i + 1);
			uint8_t value3 = *(pData + i + 2);
			uint8_t value4 = *(pData + i + 3);
			uint8_t value5 = *(pData + i + 4);
			uint8_t value6 = *(pData + i + 5);
			uint8_t value7 = *(pData + i + 6);

			//---------- dataID1[1:0] = data[1:0] = value5[1:0] ----------
			uint8_t dataID1 = 0x03 & value5;
			if (dataID1 == 0x02) //row data
			{
				//row_addr[9:0] = data_14_1[13:4] = value1[7:0] + value5[5:4]
				row = (value1 << 2) + ((0x30 & value5) >> 4);
				cout << "Package A: " << "row = " << row << endl;
			}
			else if (dataID1 == 0x01) //col data
			{
				//col_addr[10:0] = data_14_1[13:3] = value1[7:0] + value5[5:3]
				col = (value1 << 3) + ((0x38 & value5) >> 3);
				cout << "--- Package A: " << "col = " << col << endl;
				if (row >= 0 && row < CELEX5_ROW)
				{
					index = row * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] = 255;
					}
				}
				event_count++;
			}
			else if (dataID1 == 0x03) //timestamp data
			{
				//m_iLastRowTimeStamp = m_iRowTimeStamp;

				//row_time_stamp[11:0] = data_14_1[13:2] =  value1[7:0] + value5[5:2]
				//m_iRowTimeStamp = (value1 << 4) + ((0x3C & value5) >> 2);
				//cout << "Format 2 (Package_A): row_time_stamp = " << m_iRowTimeStamp << endl;

				//processMIPIEventTimeStamp();
			}

			//---------- dataID2[1:0] = data[1:0] = value5[7:6] ----------
			uint8_t dataID2 = 0xC0 & value5;
			if (dataID2 == 0x80) //row data
			{
				//row_addr[9:0] = data_14_2[13:4] = value2[7:0] + value6[3:2]
				row = (value2 << 2) + ((0x0C & value6) >> 2);
				cout << "Package B: " << "row = " << row << endl;
			}
			else if (dataID2 == 0x40) //col data
			{
				//col_addr[10:0] = data_14_2[13:3] = value2[7:0] + value6[3:1]
				col = (value2 << 3) + ((0x0E & value6) >> 1);
				//cout << "--- Package B: " << "col = " << col << endl;

				if (row >= 0 && row < CELEX5_ROW)
				{
					index = row * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] = 255;
					}
				}
				event_count++;
			}
			else if (dataID2 == 0x00) //temperature data
			{
				//temperature[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
				//temperature = (value2 << 4) + (0x0F & value6);
				//m_pCX5ProcessedData->setTemperature(temperature);
				//cout << "Format 2 time full (Package_B): temperature = " << temperature << endl;
			}
			else if (dataID2 == 0xC0) // timestamp data
			{
				//row_time_stamp[11:0] = data_14_2[13:2] = value2[7:0] + value6[3:0]
				//m_iLastRowTimeStamp = m_iRowTimeStamp;
				//m_iRowTimeStamp = (value2 << 4) + (0x0F & value6);
				//cout << "Format 2 (Package_B): row_time_stamp = " << m_iRowTimeStamp << endl;

				//processMIPIEventTimeStamp();
			}

			//---------- dataID3[1:0] = data[1:0] = value6[5:4] ----------
			uint8_t dataID3 = 0x30 & value6;
			if (dataID3 == 0x20) //row data
			{
				//row_addr[9:0] = data_14_3[13:4] = value3[7:0] + value7[1:0]
				row = (value3 << 2) + (0x03 & value7);
				cout << "Package C: " << "row = " << row << endl;
			}
			else if (dataID3 == 0x10) //col data
			{
				//col_addr[10:0] = data_14_3[13:3] = value3[7:0] + value7[1:0] + value6[7]
				col = (value3 << 3) + ((0x03 & value7) << 1) + ((0x80 & value6) >> 7);
				cout << "--- Package C: " << "col = " << col << endl;
				if (row >= 0 && row < CELEX5_ROW)
				{
					index = row * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] = 255;
					}
				}
				event_count++;
			}
			else if (dataID3 == 0x00) //temperature data
			{
				//temperature[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
				//temperature = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
				//m_pCX5ProcessedData->setTemperature(temperature);
				//cout << "Format 2 time full (Package_C): temperature = " << temperature << endl;
			}
			else if (dataID3 == 0x30) //timestamp data
			{
				//row_time_stamp[11:0] = data_14_3[13:2] = value3[7:0]+ value7[1:0] + value6[7:6]
				//m_iLastRowTimeStamp = m_iRowTimeStamp;
				//m_iRowTimeStamp = (value3 << 4) + ((0x03 & value7) << 2) + ((0xC0 & value6) >> 6);
				//cout << "Format 2 (Package_C): row_time_stamp = " << m_iRowTimeStamp << endl;

				//processMIPIEventTimeStamp();
			}

			//---------- dataID4[1:0] = data[1:0] = value7[3:2] ----------
			uint8_t dataID4 = 0x0C & value7;
			if (dataID4 == 0x08) //row data
			{
				//row_addr[9:0] = data_14_4[13:4] = value4[7:0] + value7[7:6]
				row = (value4 << 2) + ((0xC0 & value7) >> 6);
				//cout << "Package D: " << "row = " << m_iCurrentRow << endl;
			}
			else if (dataID4 == 0x04) //col data
			{
				//col_addr[10:0] = data_14_4[13:3] = value4[7:0] + value7[7:5]
				col = (value4 << 3) + ((0xE0 & value7) >> 5);
				//cout << "--- Package D: " << "col = " << col << endl;
				if (row >= 0 && row < CELEX5_ROW)
				{
					index = row * CELEX5_COL + col;
					if (index < CELEX5_PIXELS_NUMBER && index >= 0)
					{
						m_pEventCountBuffer[index] = 255;
					}
				}
				event_count++;
			}
			else if (dataID4 == 0x00) //temperature data
			{
				//temperature[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
				//temperature = (value4 << 4) + ((0xF0 & value7) >> 4);
				//m_pCX5ProcessedData->setTemperature(temperature);
				//cout << "Format 2 time full (Package_D): temperature = " << temperature << endl;
			}
			else if (dataID4 == 0x0C) //timestamp data
			{
				//row_time_stamp[11:0] = data_14_4[13:3] = value4[7:0] + value7[7:4]
				//m_iLastRowTimeStamp = m_iRowTimeStamp;
				//m_iRowTimeStamp = (value4 << 4) + ((0xF0 & value7) >> 4);
				//cout << "Format 2 (Package_D): row_time_stamp = " << m_iRowTimeStamp << endl;

				//processMIPIEventTimeStamp();
			}
		}

		//event binary pic
		cout << "event_count = " << event_count << endl;
		cv::Mat matEventPic(800, 1280, CV_8UC1, m_pEventCountBuffer);
		cv::imshow("Event Binary Pic", matEventPic);
		cv::waitKey(1000);
		cout << ffff.eof() << endl;
	}
}