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

#include <iostream>
#include <cstring>
#include <thread>
#include <algorithm>
#include "../include/celex5/celex5.h"
#include "../cx3driver/celedriver.h"
#include "../configproc/celex5cfgmgr.h"
#include "../configproc/wireincommand.h"
#include "../base/filedirectory.h"
#include "../eventproc/dataprocessthread.h"
#include "datarecorder.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#ifdef _WIN32
static HANDLE s_hEventHandle = nullptr;
#endif

using namespace std;

CeleX5::CeleX5()
	: m_bLoopModeEnabled(false)
	, m_bALSEnabled(false)
	, m_uiClockRate(100)
	, m_uiLastClockRate(100)
	, m_iEventDataFormat(2)
	, m_pDataToRead(nullptr)
	, m_uiPackageCount(0)
	, m_uiTotalPackageCount(0)
	, m_bFirstReadFinished(false)
	, m_pReadBuffer(nullptr)
	, m_emDeviceType(CeleX5::Unknown_Devive)
	, m_pCeleDriver(nullptr)
	, m_pDataProcessor(nullptr)
	, m_uiPackageCounter(0)
	, m_uiPackageTDiff(0)
	, m_uiPackageBeginT(0)
	, m_bAutoISPEnabled(false)
	, m_uiBrightness(100)
	, m_uiThreshold(171)
	, m_arrayISPThreshold{ 10, 20, 80 }
	, m_arrayBrightness{ 95, 150, 190, 200 }
	, m_uiAutoISPRefreshTime(80)
	, m_uiISOLevel(2)
	, m_uiOpticalFlowFrameTime(20)
	, m_bClockAutoChanged(false)
	, m_uiISOLevelCount(4)
	, m_iRotateType(0)
	, m_bSensorReady(false)
	, m_bShowImagesEnabled(true)
	, m_bAutoISPFrofileLoaded(false)
{
	m_pCeleX5CfgMgr = new CeleX5CfgMgr;
	//create data process thread
	m_pDataProcessThread = new DataProcessThread("CeleX5Thread");
	m_pDataProcessThread->setCeleX(this);
	m_pDataProcessor = new CeleX5DataProcessor;
	m_pDataProcessThread->setDataProcessor(m_pDataProcessor);

	m_pDataRecorder = new DataRecorder;
	//auto n = thread::hardware_concurrency();//cpu core count = 8
}

CeleX5::~CeleX5()
{
	if (m_pDataProcessThread)
	{
		if (m_pDataProcessThread->isRunning())
			m_pDataProcessThread->terminate();
		delete m_pDataProcessThread;
		m_pDataProcessThread = nullptr;
	}
	if (m_pCeleDriver)
	{
		stopSensor();
		m_pCeleDriver->clearData();
		m_pCeleDriver->closeStream();
		m_pCeleDriver->closeUSB();
		delete m_pCeleDriver;
		m_pCeleDriver = nullptr;
	}
	if (m_pCeleX5CfgMgr) delete m_pCeleX5CfgMgr;
	//
	if (m_pDataProcessor)
	{
		delete m_pDataProcessor;
		m_pDataProcessor = NULL;
	}
	if (m_pDataRecorder)
	{
		delete m_pDataRecorder;
		m_pDataRecorder = NULL;
	}
	if (m_pReadBuffer) 
	{
		delete[] m_pReadBuffer;
		m_pReadBuffer = NULL;
	}
}

/*
*  @function :  openSensor
*  @brief    :	open the celex sensor
*  @input    :  type : type of celex device
*  @output   :
*  @return   :	bool : true for open sucessfully; false for failed
*/
bool CeleX5::openSensor(DeviceType type)
{
	m_emDeviceType = type;
	if (CeleX5::CeleX5_MIPI == type)
	{
		if (NULL == m_pCeleDriver)
		{
			m_pCeleDriver = new CeleDriver;
			if (!m_pCeleDriver->openUSB())
			{
				m_pCeleX5CfgMgr->parseCeleX5Cfg(FILE_CELEX5_CFG_MIPI);
				m_mapCfgModified = m_mapCfgDefaults = getCeleX5Cfg();
				return false;
			}
		}
		FileDirectory base;
		std::string filePath = base.getApplicationDirPath();
#ifdef _WIN32
		filePath += "\\";
#endif
		filePath += FILE_CELEX5_CFG;
		if (base.isFileExists(filePath))
		{
			std::string serialNumber = getSerialNumber();
			m_pCeleX5CfgMgr->parseCeleX5Cfg(FILE_CELEX5_CFG);
			if (serialNumber.size() > 4 && serialNumber.at(4) == 'M') //no wire version
			{
				m_uiISOLevel = 2;
				m_uiISOLevelCount = 4;
				m_uiBrightness = 130;
			}
			else //wire version
			{
				m_uiISOLevel = 3;
				m_uiISOLevelCount = 6;
				m_uiBrightness = 100;
			}
			m_mapCfgModified = m_mapCfgDefaults = getCeleX5Cfg();
		}
		else
		{
			std::ofstream out(filePath);
			out.close();
			std::string serialNumber = getSerialNumber();
			if (serialNumber.size() > 4 && serialNumber.at(4) == 'M') //no wire version
			{
				m_pCeleX5CfgMgr->parseCeleX5Cfg(FILE_CELEX5_CFG_MIPI);
				m_pCeleX5CfgMgr->saveCeleX5XML(FILE_CELEX5_CFG_MIPI);
				m_uiISOLevel = 2;
				m_uiISOLevelCount = 4;
				m_uiBrightness = 130;
			}
			else //wire version
			{
				m_pCeleX5CfgMgr->parseCeleX5Cfg(FILE_CELEX5_CFG_MIPI_WRIE);
				m_pCeleX5CfgMgr->saveCeleX5XML(FILE_CELEX5_CFG_MIPI_WRIE);
				m_uiISOLevel = 3;
				m_uiISOLevelCount = 6;
				m_uiBrightness = 100;
			}
			m_mapCfgModified = m_mapCfgDefaults = getCeleX5Cfg();
			m_pCeleX5CfgMgr->saveCeleX5XML(m_mapCfgDefaults);
		}

		if (!configureSettings(type))
			return false;

		setISOLevel(m_uiISOLevel);
		//m_pDataProcessor->setISOLevel(m_uiISOLevel);

		clearData();
		m_pDataProcessThread->start();
	}
	m_bSensorReady = true;
	return true;
}

/*
*  @function :  isSensorReady
*  @brief    :	the state of the celex sensor
*  @input    :
*  @output   :
*  @return   :	bool : true for open sucessfully; false for failed
*/
bool CeleX5::isSensorReady()
{
	return m_bSensorReady;
}

/*
*  @function :  getCeleXRawData
*  @brief    :	interface for getting MIPI format data from cypress
*  @input    :	buffer : the vector for saving raw image buffer
*  @output   :
*  @return   :
*/
void CeleX5::getCeleXRawData(uint8_t* pData, uint32_t& length)
{
	if (CeleX5::CeleX5_MIPI != m_emDeviceType)
	{
		return;
	}
	m_pCeleDriver->getSensorData(pData, length);
}

/*
*  @function :  getCeleXRawData
*  @brief    :	interface for getting MIPI format data, time stamps and raw IMU data
*  @input    :	buffer : the vector for saving raw image buffer
*				timeStampEnd : the end time stamp of the buffer
*				imuData : the vector for saving raw IMU buffer
*  @output   :
*  @return   :
*/
void CeleX5::getCeleXRawData(uint8_t* pData, uint32_t& length, std::time_t& timestampEnd, std::vector<IMURawData>& imu_data)
{
	if (CeleX5::CeleX5_MIPI != m_emDeviceType)
	{
		return;
	}
	std::vector<IMURawData> imu_raw_data;
	if (m_pCeleDriver->getSensorData(pData, length, timestampEnd, imu_raw_data))
	{
		imu_data = std::vector<IMURawData>(imu_raw_data.size());
		for (int i = 0; i < imu_raw_data.size(); i++)
		{
			//cout << "--------------"<<imu_raw_data[i].time_stamp << endl;
			memcpy(imu_data[i].imuData, imu_raw_data[i].imuData, sizeof(imu_raw_data[i].imuData));
			imu_data[i].timestamp = imu_raw_data[i].timestamp;
			//imu_data[i] = ((struct IMURawData*)&imu_raw_data[i]);
		}
		//record sensor data
		if (m_pDataRecorder->isRecording())
		{
			m_pDataRecorder->writeData(pData, length, timestampEnd, imu_data);
		}

		//calculate the package count per second
		if (!m_bLoopModeEnabled && getSensorFixedMode() > 2)
		{
			m_uiPackageCounter++;
#ifdef _WIN32
			uint32_t t2 = GetTickCount();
#else
			uint32_t t2 = clock() / 1000;
#endif
			m_uiPackageTDiff += (t2 - m_uiPackageBeginT);
			m_uiPackageBeginT = t2;
			if (m_uiPackageTDiff > 1000)
			{
				//cout << "--- package count = " << m_uiPackageCounter << endl;
				m_pDataProcessor->getProcessedData()->setFullFrameFPS(m_uiPackageCountPS);
				m_uiPackageTDiff = 0;
				m_uiPackageCountPS = m_uiPackageCounter;
				m_uiPackageCounter = 0;
			}
		}
	}
}

/*
*  @function :  parseCeleXRawData
*  @brief    :	interface for parsing MIPI format data
*  @input    :	pData : the data for parsing
*				dataSize : the size of pData buffer
*  @output   :
*  @return   :
*/
void CeleX5::parseCeleXRawData(uint8_t* pData, uint32_t dataSize)
{
	std::vector<IMURawData> imu_data;
	m_pDataProcessor->processMIPIData(pData, dataSize, 0, imu_data);
}

/*
*  @function :  parseCeleXRawData
*  @brief    :	interface for parsing MIPI format data
*  @input    :	pData : the data for parsing
*				dataSize : the size of pData buffer
*				timeStampEnd : the end time stamp of the buffer
*				imuData : the vector for saving raw IMU buffer
*  @output   :
*  @return   :
*/
void CeleX5::parseCeleXRawData(uint8_t* pData, uint32_t dataSize, std::time_t timestampEnd, std::vector<IMURawData> imuData)
{
	m_pDataProcessor->processMIPIData(pData, dataSize, timestampEnd, imuData);
}

/*
*  @function :  disableFrameModule
*  @brief    :	disable the frame creating module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableFrameModule()
{
	m_pDataProcessor->disableFrameModule();
}

/*
*  @function :  enableFrameModule
*  @brief    :	enable the frame creating module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableFrameModule()
{
	m_pDataProcessor->enableFrameModule();
}

/*
*  @function :  isFrameModuleEnabled
*  @brief    :	get the state of frame module
*  @input    :
*  @output   :
*  @return   :  bool : true for frame module enable; false for disabled
*/
bool CeleX5::isFrameModuleEnabled()
{
	return m_pDataProcessor->isFrameModuleEnabled();
}

/*
*  @function :  disableEventStreamModule
*  @brief    :	disable the event stream module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableEventStreamModule()
{
	m_pDataProcessor->disableEventStreamModule();
}

/*
*  @function :  enableEventStreamModule
*  @brief    :	enable the event stream module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableEventStreamModule()
{
	m_pDataProcessor->enableEventStreamModule();
}

/*
*  @function :  isEventStreamEnabled
*  @brief    :	get the state of event stream module
*  @input    :
*  @output   :
*  @return   :  bool : true for event stream module enable; false for disabled
*/
bool CeleX5::isEventStreamEnabled()
{
	return m_pDataProcessor->isEventStreamEnabled();
}

/*
*  @function :  disableIMUModule
*  @brief    :	disable the IMU module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableIMUModule()
{
	//imu_enable: bit[8], hard_reset: bit[3:2], als_enable: bit[1]
	//als: bit[1] = 1 menas disabled; bit[1] = 0 means enabled
	if (m_bALSEnabled)
		m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0000);
	else
		m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0002);

	m_pDataProcessor->disableIMUModule();
}

/*
*  @function :  enableIMUModule
*  @brief    :	enable the IMU module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableIMUModule()
{
	//imu_enable: bit[8], hard_reset: bit[3:2], als_enable: bit[1]
	if (m_bALSEnabled)
		m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0100);
	else
		m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0102);

	m_pDataProcessor->enableIMUModule();
}

/*
*  @function :  isIMUModuleEnabled
*  @brief    :	get the state of IMU module
*  @input    :
*  @output   :
*  @return   :  bool : true for IMU module enable; false for disabled
*/
bool CeleX5::isIMUModuleEnabled()
{
	return m_pDataProcessor->isIMUModuleEnabled();
}

/*
*  @function :  disableEventDenoising
*  @brief    :	disable the event denoise module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableEventDenoising()
{
	m_pDataProcessor->disableEventDenoising();
}

/*
*  @function :  enableEventDenoising
*  @brief    :	enable the event denoise module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableEventDenoising()
{
	m_pDataProcessor->enableEventDenoising();
}

/*
*  @function :  isEventDenoisingEnabled
*  @brief    :	get the state of event denoise module
*  @input    :
*  @output   :
*  @return   :  bool : true for event denoise module enable; false for disabled
*/
bool CeleX5::isEventDenoisingEnabled()
{
	return m_pDataProcessor->isEventDenoisingEnabled();
}

/*
*  @function :  disableFrameDenoising
*  @brief    :	disable the frame denoise module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableFrameDenoising()
{
	m_pDataProcessor->disableFrameDenoising();
}

/*
*  @function :  enableFrameDenoising
*  @brief    :	enable the frame denoise module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableFrameDenoising()
{
	m_pDataProcessor->enableFrameDenoising();
}

/*
*  @function :  isFrameDenoisingEnabled
*  @brief    :	get the state of frame denoise module
*  @input    :
*  @output   :
*  @return   :  bool : true for frame denoise module enable; false for disabled
*/
bool CeleX5::isFrameDenoisingEnabled()
{
	return m_pDataProcessor->isFrameDenoisingEnabled();
}

/*
*  @function :  disableEventCountSlice
*  @brief    :	disable the event count slice module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableEventCountSlice()
{
	m_pDataProcessor->disableEventCountSlice();
}

/*
*  @function :  enableEventCountSlice
*  @brief    :	enable the event count slice module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableEventCountSlice()
{
	m_pDataProcessor->enableEventCountSlice();;
}

/*
*  @function :  isEventCountSliceEnabled
*  @brief    :	get the state of event count slice module
*  @input    :
*  @output   :
*  @return   :  bool : true for event count slice module enable; false for disabled
*/
bool CeleX5::isEventCountSliceEnabled()
{
	return m_pDataProcessor->isEventCountSliceEnabled();
}


/*
*  @function :  disableEventOpticalFlow
*  @brief    :	disable the event optical-flow module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableEventOpticalFlow()
{
	m_pDataProcessor->disableEventOpticalFlow();
}

/*
*  @function :  enableEventOpticalFlow
*  @brief    :	enable the event optical-flow module
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableEventOpticalFlow()
{
	m_pDataProcessor->enableEventOpticalFlow();;
}

/*
*  @function :  isEventOpticalFlowEnabled
*  @brief    :	get the state of event optical-flow
*  @input    :
*  @output   :
*  @return   :  bool : true for event optical-flow module enable; false for disabled
*/
bool CeleX5::isEventOpticalFlowEnabled()
{
	return m_pDataProcessor->isEventOpticalFlowEnabled();
}

/*
*  @function :  getFullPicBuffer
*  @brief    :	get the full buffer of sensor
*  @input    :  buffer: the buffer pointer for full frame
*  @output   :	
*  @return   :
*/
void CeleX5::getFullPicBuffer(uint8_t* buffer)
{
	m_pDataProcessor->getFullPicBuffer(buffer);
}

/*
*  @function :  getFullPicBuffer
*  @brief    :	get the full buffer of sensor
*  @input    :  buffer: the buffer pointer for full frame
*  @output   :	timeStamp: the time stamp of the full frame
*  @return   :
*/
void CeleX5::getFullPicBuffer(uint8_t* buffer, std::time_t& timestamp)
{
	m_pDataProcessor->getFullPicBuffer(buffer, timestamp);
}

/*
*  @function :  getFullPicMat
*  @brief    :	get the full pic mat of sensor
*  @input    :
*  @output   :	
*  @return   :	the mat format full pic
*/
cv::Mat CeleX5::getFullPicMat()
{
	//CeleX5ProcessedData* pSensorData = m_pDataProcessor->getProcessedData();
	//if (pSensorData)
	//{
	//	return cv::Mat(cv::Size(CELEX5_COL, CELEX5_ROW), CV_8UC1, pSensorData->getFullPicBuffer());
	//}
	//return cv::Mat();

	cv::Mat matPic(CELEX5_ROW, CELEX5_COL, CV_8UC1);
	m_pDataProcessor->getFullPicBuffer(matPic.data);
	return matPic;
}

/*
*  @function :  getEventPicBuffer
*  @brief    :	get the event buffer of sensor
*  @input    :
*  @output   :	buffer : the buffer pointer for event frame
*				type : the type the event frame
*  @return   :
*/
void CeleX5::getEventPicBuffer(uint8_t* buffer, EventPicType type)
{
	m_pDataProcessor->getEventPicBuffer(buffer, type);
}

/*
*  @function :  getEventPicBuffer
*  @brief    :	get the event buffer of sensor
*  @input    :
*  @output   :	buffer : the buffer pointer for event frame
*				timeStamp : the time stamp of the event frame
*				type : the type the event frame
*  @return   :
*/
void CeleX5::getEventPicBuffer(uint8_t* buffer, std::time_t& timestamp, EventPicType type)
{
	m_pDataProcessor->getEventPicBuffer(buffer, timestamp, type);
}

/*
*  @function :  getEventPicMat
*  @brief    :	get the event mat of sensor
*  @input    :
*  @output   :	type : the type the event frame
*  @return   :	the mat format event pic
*/
cv::Mat CeleX5::getEventPicMat(EventPicType type)
{
	//CeleX5ProcessedData* pSensorData = m_pDataProcessor->getProcessedData();
	//if (pSensorData)
	//{
	//	return cv::Mat(cv::Size(CELEX5_COL, CELEX5_ROW), CV_8UC1, pSensorData->getEventPicBuffer(type));
	//}
	//return cv::Mat();

	cv::Mat matPic(CELEX5_ROW, CELEX5_COL, CV_8UC1);
	m_pDataProcessor->getEventPicBuffer(matPic.data, type);
	return matPic;
}

/*
*  @function :  getOpticalFlowPicBuffer
*  @brief    :	get the optical flow buffer of sensor
*  @input    :
*  @output   :	buffer : the buffer pointer for optical flow frame
*				type : the type the optical flow frame
*  @return   :
*/
void CeleX5::getOpticalFlowPicBuffer(uint8_t* buffer, OpticalFlowPicType type)
{
	m_pDataProcessor->getOpticalFlowPicBuffer(buffer, type);
}

void CeleX5::getOpticalFlowPicBuffer(uint8_t* buffer, std::time_t& timestamp, OpticalFlowPicType type/* = Optical_Flow_Pic*/)
{
	m_pDataProcessor->getOpticalFlowPicBuffer(buffer, timestamp, type);
}

/*
*  @function :  getOpticalFlowPicMat
*  @brief    :	get the event mat of sensor
*  @input    :
*  @output   :	type : the type the optical flow frame
*  @return   :	the mat format optical flow pic
*/
cv::Mat CeleX5::getOpticalFlowPicMat(OpticalFlowPicType type)
{
	//CeleX5ProcessedData* pSensorData = m_pDataProcessor->getProcessedData();
	//if (pSensorData)
	//{
	//	return cv::Mat(cv::Size(CELEX5_COL, CELEX5_ROW), CV_8UC1, pSensorData->getOpticalFlowPicBuffer(type));
	//}
	//return cv::Mat();

	cv::Mat matPic(CELEX5_ROW, CELEX5_COL, CV_8UC1);
	m_pDataProcessor->getOpticalFlowPicBuffer(matPic.data, type);
	return matPic;
}

/*
*  @function :  getEventDataVector
*  @brief    :	get the vector of the event data
*  @input    :
*  @output   :	vecEvent: the vector of the event data
*  @return   :	bool : true for non-empty vector; false for empty vector
*/
bool CeleX5::getEventDataVector(std::vector<EventData>& vecEvent)
{
	return m_pDataProcessor->getEventDataVector(vecEvent);
}

/*
*  @function :  getEventDataVector
*  @brief    :	get the vector of the event data
*  @input    :
*  @output   :	vecEvent: the vector of the event data
*				frameNo : the frame number of the event vector
*  @return   :	bool : true for non-empty vector; false for empty vector
*/
bool CeleX5::getEventDataVector(std::vector<EventData>& vecEvent, uint32_t& frameNo)
{
	return m_pDataProcessor->getEventDataVector(vecEvent, frameNo);
}

/*
*  @function :  getEventDataVectorEx
*  @brief    :	get the vector of the event data
*  @input    :
*  @output   :	vecEvent : the vector of the event data
*				frameNo  : the frame number of the event vector
*               timestamp: the pc timestamp when the event data stream was received from sensor
*  @return   :	bool : true for non-empty vector; false for empty vector
*/
bool CeleX5::getEventDataVector(std::vector<EventData>& vecEvent, uint32_t& frameNo, std::time_t& timestamp)
{
	return m_pDataProcessor->getEventDataVector(vecEvent, frameNo, timestamp);
}

/*
*  @function :  getIMUData
*  @brief    :	get the vector of the IMU data
*  @input    :
*  @output   :	data : the vector of the IMU data
*  @return   :	The number of IMU data packets actually obtained.
*/
int CeleX5::getIMUData(std::vector<IMUData>& data)
{
	return m_pDataProcessor->getIMUData(data);
}

/*
*  @function :  setSensorFixedMode
*  @brief    :	set the Sensor operation mode in fixed mode
*  @input    :
*  @output   :	mode : the fixed working mode of CeleX-5 sensor
*  @return   :	the number of IMU data packets actually obtained.
*/
void CeleX5::setSensorFixedMode(CeleX5Mode mode)
{
	if (CeleX5::Event_Intensity_Mode == mode || CeleX5::Event_In_Pixel_Timestamp_Mode == mode)
	{
		if (m_uiClockRate > 70)
		{
			m_bClockAutoChanged = true;
			m_uiLastClockRate = m_uiClockRate;
			setClockRate(70);
		}
	}
	else
	{
		if (m_bClockAutoChanged)
		{
			setClockRate(m_uiLastClockRate);
			m_bClockAutoChanged = false;
		}
	}

	m_pDataProcessThread->clearData();
	m_pCeleDriver->clearData();

	//Disable ALS read and write, must be the first operation
	setALSEnabled(false);

	//Enter CFG Mode
	enterCFGMode();

	//Write Fixed Sensor Mode
	wireIn(SENSOR_MODE_1, static_cast<uint32_t>(mode), 0xFF);

	//Disable brightness adjustment (auto isp), always load sensor core parameters from profile0
	wireIn(AUTOISP_BRT_EN, 0, 0xFF); //AUTOISP_BRT_EN
	wireIn(AUTOISP_TRIGGER, 0, 0xFF); //AUTOISP_TRIGGER
	wireIn(AUTOISP_PROFILE_ADDR, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
	writeRegister(AUTOISP_BRT_VALUE_H, -1, AUTOISP_BRT_VALUE_L, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
	writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_uiBrightness); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0

	if (CeleX5::Event_Intensity_Mode == mode || CeleX5::Event_In_Pixel_Timestamp_Mode == mode)
	{
		m_iEventDataFormat = 1;
		wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
		m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);
		/*
		(1) CSR_114 / CSR_115 = 96
		(2) CSR_74 = 254
		(3) CSR_76 / CSR_77 = 1280

		(4) CSR_79 / CSR_80 = 400
		(5) CSR_82 / CSR_83 = 800
		(6) CSR_84 / CSR_85 = 462
		(7) CSR_86 / CSR_87 = 1200
		*/
		writeRegister(MIPI_ROW_NUM_EVENT_H, -1, MIPI_ROW_NUM_EVENT_L, 200);
		writeRegister(MIPI_HD_GAP_FULLFRAME_H, -1, MIPI_HD_GAP_FULLFRAME_L, 800);
		writeRegister(MIPI_HD_GAP_EVENT_H, -1, MIPI_HD_GAP_EVENT_L, 462);
		writeRegister(MIPI_GAP_EOF_SOF_H, -1, MIPI_GAP_EOF_SOF_L, 1200);
	}
	else if (CeleX5::Event_Off_Pixel_Timestamp_Mode == mode)
	{
		m_iEventDataFormat = 2;
		wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
		m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);
		/*
		(1) CSR_114 / CSR_115 = 96
		(2) CSR_74 = 254
		(3) CSR_76 / CSR_77 = 1280

		(4) CSR_79 / CSR_80 = 200
		(5) CSR_82 / CSR_83 = 720
		(6) CSR_84 / CSR_85 = 680
		(7) CSR_86 / CSR_87 = 1300
		*/
		writeRegister(MIPI_ROW_NUM_EVENT_H, -1, MIPI_ROW_NUM_EVENT_L, 200);
		writeRegister(MIPI_HD_GAP_FULLFRAME_H, -1, MIPI_HD_GAP_FULLFRAME_L, 720);
		writeRegister(MIPI_HD_GAP_EVENT_H, -1, MIPI_HD_GAP_EVENT_L, 680);
		writeRegister(MIPI_GAP_EOF_SOF_H, -1, MIPI_GAP_EOF_SOF_L, 1300);
	}

	if (CeleX5::Event_In_Pixel_Timestamp_Mode == mode ||
		CeleX5::Optical_Flow_Mode == mode ||
		CeleX5::Multi_Read_Optical_Flow_Mode == mode ||
		CeleX5::Optical_Flow_FPN_Mode == mode)
	{
		//wireIn(45, 1, 0xFF);
		std::vector<CfgInfo> cfgSensorCoreParameters = m_mapCfgDefaults["Sensor_Core_Parameters"];
		CfgInfo cfg_col_gain = cfgSensorCoreParameters.at(23);
		wireIn(45, 1, cfg_col_gain.value);
		//cout << "cfg_col_gain.value = " << cfg_col_gain.value << endl;
	}

	std::vector<CfgInfo> cfgSensorCoreParameters = m_mapCfgDefaults["Sensor_Core_Parameters"];
	CfgInfo cfg_bias_rampn = cfgSensorCoreParameters.at(7);
	CfgInfo cfg_bias_rampp = cfgSensorCoreParameters.at(8);
	if (CeleX5::Optical_Flow_FPN_Mode == mode)
	{
		//writeRegister(BIAS_RAMPP_H, -1, BIAS_RAMPP_L, 530);
		writeRegister(BIAS_RAMPP_H, -1, BIAS_RAMPP_L, cfg_bias_rampn.value + (cfg_bias_rampp.value - cfg_bias_rampn.value) / 2);
	}
	else
	{
		//writeRegister(BIAS_RAMPP_H, -1, BIAS_RAMPP_L, 735);
		writeRegister(BIAS_RAMPP_H, -1, BIAS_RAMPP_L, cfg_bias_rampp.value);
	}

	//Enter Start Mode
	enterStartMode();

	if (CeleX5::Event_Intensity_Mode == mode ||
		CeleX5::Full_Picture_Mode == mode)
	{
		setISOLevel(m_uiISOLevel);
	}

	m_pDataProcessor->setSensorFixedMode(mode);
}

/*
*  @function :  getSensorFixedMode
*  @brief    :	get the sensor operation mode in fixed mode
*  @input    :
*  @output   :
*  @return   :	the fixed working mode of CeleX-5 sensor
*/
CeleX5::CeleX5Mode CeleX5::getSensorFixedMode()
{
	return m_pDataProcessor->getSensorFixedMode();
}

/*
*  @function :  setSensorLoopMode
*  @brief    :	set the sensor operation mode in loop mode
*  @input    :	mode :  the working mode of CeleX-5 sensor
*				loopNum : the number of the loop
*				loopNum = 1: the first operation mode in loop mode, address = 53, width = [2:0]
*				loopNum = 2: the second operation mode in loop mode, address = 54, width = [2:0]
*				loopNum = 3: the third operation mode in loop mode, address = 55, width = [2:0]
*  @output   :
*  @return   :
*/
void CeleX5::setSensorLoopMode(CeleX5Mode mode, int loopNum)
{
	if (loopNum < 1 || loopNum > 3)
	{
		std::cout << "CeleX5::setSensorMode: wrong loop number!" << std::endl;
		return;
	}
	clearData();

	if (getSensorLoopMode(1) == CeleX5::Event_Intensity_Mode || getSensorLoopMode(1) == CeleX5::Event_In_Pixel_Timestamp_Mode ||
		getSensorLoopMode(2) == CeleX5::Event_Intensity_Mode || getSensorLoopMode(2) == CeleX5::Event_In_Pixel_Timestamp_Mode ||
		getSensorLoopMode(3) == CeleX5::Event_Intensity_Mode || getSensorLoopMode(3) == CeleX5::Event_In_Pixel_Timestamp_Mode)
	{
		if (m_uiClockRate != 70)
			setClockRate(70);
	}
	else
	{
		if (m_uiClockRate != 100)
			setClockRate(100);
	}
	/*if (CeleX5::Event_Intensity_Mode == mode || CeleX5::Event_Optical_Flow_Mode == mode)
	{
	if (m_uiClockRate > 70)
	{
	m_bClockAutoChanged = true;
	m_uiLastClockRate = m_uiClockRate;
	setClockRate(70);
	}
	}
	else
	{
	if (m_bClockAutoChanged)
	{
	setClockRate(m_uiLastClockRate);
	m_bClockAutoChanged = false;
	}
	}*/
	enterCFGMode();
	wireIn(SENSOR_MODE_1 + loopNum - 1, static_cast<uint32_t>(mode), 0xFF);

	if (CeleX5::Event_Intensity_Mode == mode || CeleX5::Event_In_Pixel_Timestamp_Mode == mode)
	{
		m_iEventDataFormat = 1;
		wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
		m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);

		writeRegister(MIPI_ROW_NUM_EVENT_H, -1, MIPI_ROW_NUM_EVENT_L, 200);
		writeRegister(MIPI_HD_GAP_FULLFRAME_H, -1, MIPI_HD_GAP_FULLFRAME_L, 800);
		writeRegister(MIPI_HD_GAP_EVENT_H, -1, MIPI_HD_GAP_EVENT_L, 462);
		writeRegister(MIPI_GAP_EOF_SOF_H, -1, MIPI_GAP_EOF_SOF_L, 1200);
	}
	else if (CeleX5::Event_Off_Pixel_Timestamp_Mode == mode)
	{
		int loopNum1 = loopNum + 1;
		if (loopNum1 > 3)
			loopNum1 = loopNum1 % 3;
		int loopNum2 = loopNum + 2;
		if (loopNum2 > 3)
			loopNum2 = loopNum2 % 3;

		if (m_pDataProcessor->getSensorLoopMode(loopNum1) != CeleX5::Event_Intensity_Mode &&
			m_pDataProcessor->getSensorLoopMode(loopNum1) != CeleX5::Event_In_Pixel_Timestamp_Mode &&
			m_pDataProcessor->getSensorLoopMode(loopNum2) != CeleX5::Event_Intensity_Mode &&
			m_pDataProcessor->getSensorLoopMode(loopNum2) != CeleX5::Event_In_Pixel_Timestamp_Mode)
		{
			m_iEventDataFormat = 2;
			wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
			m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);

			writeRegister(MIPI_ROW_NUM_EVENT_H, -1, MIPI_ROW_NUM_EVENT_L, 200);
			writeRegister(MIPI_HD_GAP_FULLFRAME_H, -1, MIPI_HD_GAP_FULLFRAME_L, 720);
			writeRegister(MIPI_HD_GAP_EVENT_H, -1, MIPI_HD_GAP_EVENT_L, 680);
			writeRegister(MIPI_GAP_EOF_SOF_H, -1, MIPI_GAP_EOF_SOF_L, 1300);
		}
	}
	enterStartMode();
	m_pDataProcessor->setSensorLoopMode(mode, loopNum);
}

/*
*  @function :  getSensorLoopMode
*  @brief    :	get the sensor operation mode in loop mode
*  @input    :	loopNum : the number of the loop
*  @output   :
*  @return   :	the loop working mode of CeleX-5 sensor
*/
CeleX5::CeleX5Mode CeleX5::getSensorLoopMode(int loopNum)
{
	return m_pDataProcessor->getSensorLoopMode(loopNum);
}

/*
*  @function :  setLoopModeEnabled
*  @brief    :	enable or disable the loop mode of the sensor
*  @input    :	enable : true for enable, false for disable
*  @output   :
*  @return   :
*/
void CeleX5::setLoopModeEnabled(bool enable)
{
	m_bLoopModeEnabled = enable;

	if (CeleX5::CeleX5_MIPI == m_emDeviceType)
	{
		if (m_bAutoISPEnabled)
			setALSEnabled(false);
	}
	bool bChangeParameters = false;
	if (enable)
	{
		if (m_bClockAutoChanged)
		{
			setClockRate(m_uiLastClockRate);
			m_bClockAutoChanged = false;
			bChangeParameters = true;
		}
	}
	enterCFGMode();
	if (enable)
	{
		wireIn(SENSOR_MODE_SELECT, 1, 0xFF);

		if (CeleX5::CeleX5_MIPI == m_emDeviceType)
		{
			//Disable brightness adjustment (auto isp), always load sensor core parameters from profile0
			wireIn(AUTOISP_BRT_EN, 0, 0xFF); //AUTOISP_BRT_EN, disable auto isp
			wireIn(AUTOISP_TRIGGER, 1, 0xFF); //AUTOISP_TRIGGER
			wireIn(AUTOISP_PROFILE_ADDR, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
			writeRegister(AUTOISP_BRT_VALUE_H, -1, AUTOISP_BRT_VALUE_L, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
			writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_uiBrightness); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0
		}

		if (bChangeParameters)
		{
			m_iEventDataFormat = 2;
			wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
			m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);
			/*
			(1) CSR_114 / CSR_115 = 96
			(2) CSR_74 = 254
			(3) CSR_76 / CSR_77 = 1280

			(4) CSR_79 / CSR_80 = 200
			(5) CSR_82 / CSR_83 = 720
			(6) CSR_84 / CSR_85 = 680
			(7) CSR_86 / CSR_87 = 1300
			*/
			writeRegister(MIPI_ROW_NUM_EVENT_H, -1, MIPI_ROW_NUM_EVENT_L, 200);
			writeRegister(MIPI_HD_GAP_FULLFRAME_H, -1, MIPI_HD_GAP_FULLFRAME_L, 720);
			writeRegister(MIPI_HD_GAP_EVENT_H, -1, MIPI_HD_GAP_EVENT_L, 680);
			writeRegister(MIPI_GAP_EOF_SOF_H, -1, MIPI_GAP_EOF_SOF_L, 1300);
		}
	}
	else
	{
		wireIn(SENSOR_MODE_SELECT, 0, 0xFF);
	}
	enterStartMode();
	m_pDataProcessor->setLoopModeEnabled(enable);
}

/*
*  @function :  isLoopModeEnabled
*  @brief    :	the state whether the Loop Mode is enabled.
*  @input    :
*  @output   :
*  @return   :	the state whether the Loop Mode is enabled.
*/
bool CeleX5::isLoopModeEnabled()
{
	return m_bLoopModeEnabled;
}

/*
*  @function :  setFpnFile
*  @brief    :	set the FPN path
*  @input    :	fpnFile : the directory path and file name of FPN file required
*  @output   :
*  @return   :	bool : true for openning fpn file successfully ; false for failed
*/
bool CeleX5::setFpnFile(const std::string& fpnFile)
{
	return m_pDataProcessor->setFpnFile(fpnFile);
}

/*
*  @function :  generateFPN
*  @brief    :	generate the FPN path
*  @input    :	fpnFile : the directory path and file name of FPN file to be saved
*  @output   :
*  @return   :
*/
void CeleX5::generateFPN(const std::string& fpnFile)
{
	if (getSensorFixedMode() == CeleX5::Event_In_Pixel_Timestamp_Mode)
	{
		enterCFGMode();
		writeRegister(14, -1, 15, 680);
		writeRegister(16, -1, 17, 680);
		enterStartMode();
	}
	m_pDataProcessor->generateFPN(fpnFile);
}

void CeleX5::stopGenerateFPN()
{
	enterCFGMode();
	vector<CfgInfo> cfgSensorCoreParameters = m_mapCfgDefaults["Sensor_Core_Parameters"];
	CfgInfo cfg_bias_rampn = cfgSensorCoreParameters.at(7);
	CfgInfo cfg_bias_rampp = cfgSensorCoreParameters.at(8);	
	writeRegister(14, -1, 15, cfg_bias_rampn.value);
	writeRegister(16, -1, 17, cfg_bias_rampp.value);
	enterStartMode();
}

/*
*  @function :  setClockRate
*  @brief    :	set the clock rate of the sensor
*  @input    :	value : the clock rate of the CeleX-5 sensor, unit is MHz
*  @output   :
*  @return   :
*/
void CeleX5::setClockRate(uint32_t value)
{
	if (value > 100 || value < 20)
		return;
	m_uiClockRate = value;
	if (CeleX5::CeleX5_MIPI == m_emDeviceType)
	{
		if (value < 70)
		{
			m_bClockAutoChanged = false;
		}
		enterCFGMode();

		// Disable PLL
		wireIn(PLL_PD_B, 0, 0xFF);
		int clock[15] = { 20,  30,  40,  50,  60,  70,  80,  90, 100, 110, 120, 130, 140, 150, 160 };

		int _PLL_DIV_N[15] = { 12,  18,  12,  15,  18,  21,  12,  18,  15,  22,  18,  26,  21,  30, 24 };
		int _PLL_DIV_L[15] = { 2,   3,   2,   2,   2,   2,   2,   3,   2,   3,   2,   3,   2,   3,  2 };
		int _PLL_FOUT_DIV1[15] = { 3,   2,   1,   1,   1,   1,   0,   0,   0,   0,   0,   0,   0,   0,  0 };
		int _PLL_FOUT_DIV2[15] = { 3,   2,   3,   3,   3,   3,   3,   2,   3,   3,   3,   3,   3,   3,  3 };

		int _MIPI_PLL_DIV_I[15] = { 3,   2,   3,   3,   2,   2,   3,   2,   3,   2,   2,   2,   2,   2,  1 };
		int MIPI_PLL_DIV_N[15] = { 120, 120, 120, 96,  120, 102, 120, 120, 96,  130, 120, 110, 102, 96, 120 };

		int index = value / 10 - 2;

		std::cout << "CeleX5::setClockRate: " << clock[index] << " MHz" << std::endl;

		// Write PLL Clock Parameter
		writeRegister(PLL_DIV_N, -1, -1, _PLL_DIV_N[index]);
		writeRegister(PLL_DIV_L, -1, -1, _PLL_DIV_L[index]);
		writeRegister(PLL_FOUT_DIV1, -1, -1, _PLL_FOUT_DIV1[index]);
		writeRegister(PLL_FOUT_DIV2, -1, -1, _PLL_FOUT_DIV2[index]);

		// Enable PLL
		wireIn(PLL_PD_B, 1, 0xFF);

		disableMIPI();
		writeRegister(MIPI_PLL_DIV_I, -1, -1, _MIPI_PLL_DIV_I[index]);
		writeRegister(MIPI_PLL_DIV_N_H, -1, MIPI_PLL_DIV_N_L, MIPI_PLL_DIV_N[index]);
		enableMIPI();

		if (value > 100)
		{
			writeRegister(84, -1, 85, 900);
			writeRegister(86, -1, 87, 1000);
		}

		enterStartMode();

		uint32_t frame_time = m_pDataProcessor->getEventFrameTime();
		m_pDataProcessor->setEventFrameTime(frame_time, m_uiClockRate);
	}
}

/*
*  @function :  getClockRate
*  @brief    :	get the clock rate of the sensor
*  @input    :
*  @output   :
*  @return   :	the clock rate of the CeleX-5 sensor, unit is MHz
*/
uint32_t CeleX5::getClockRate()
{
	return m_uiClockRate;
}

/*
*  @function :  setThreshold
*  @brief    :	set the threshold of the sensor
*  @input    :	value : threshold value
*  @output   :
*  @return   :
*/
void CeleX5::setThreshold(uint32_t value)
{
	m_uiThreshold = value;

	enterCFGMode();

	int EVT_VL = 512 - value;
	if (EVT_VL < 0)
		EVT_VL = 0;
	writeRegister(2, -1, 3, EVT_VL);

	int EVT_VH = 512 + value;
	if (EVT_VH > 1023)
		EVT_VH = 1023;
	writeRegister(6, -1, 7, EVT_VH);

	enterStartMode();
}

/*
*  @function :  getThreshold
*  @brief    :	get the threshold of the sensor
*  @input    :
*  @output   :
*  @return   :	the threshold of the CeleX-5 sensor
*/
uint32_t CeleX5::getThreshold()
{
	return m_uiThreshold;
}

/*
*  @function :  setBrightness
*  @brief    :	set the brightness of the sensor
*  @input    :	value : brightness value
*  @output   :
*  @return   :
*/
void CeleX5::setBrightness(uint32_t value)
{
	m_uiBrightness = value;

	enterCFGMode();
	writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, value);
	enterStartMode();
}

/*
*  @function :  getBrightness
*  @brief    :	get the brightness of the sensor
*  @input    :
*  @output   :
*  @return   :	the brightness of the CeleX-5 sensor
*/
uint32_t CeleX5::getBrightness()
{
	return m_uiBrightness;
}

/*
*  @function :  setISOLevel
*  @brief    :	set the ISO level of the sensor
*  @input    :	value : ISO level, 1-6 for sensor with jumper wire, 1-4 for sensor without jumper wire
*  @output   :
*  @return   :
*/
void CeleX5::setISOLevel(uint32_t value)
{
	if (6 == m_uiISOLevelCount)
	{
		int index = value - 1;
		if (index < 0)
			index = 0;
		if (index > 5)
			index = 5;

		m_uiISOLevel = index + 1;
		m_pDataProcessor->setISOLevel(m_uiISOLevel);

		int col_gain[6] = { 2, 2, 2, 2, 1, 1 };
		int bias_advl_i[6] = { 470, 410, 350, 290, 410, 380 };
		int bias_advh_i[6] = { 710, 770, 830, 890, 770, 800 };
		int bias_advcl_i[6] = { 560, 545, 530, 515, 545, 540 };
		int bias_advch_i[6] = { 620, 635, 650, 665, 635, 640 };
		int bias_vcm_i[6] = { 590, 590, 590, 590, 590, 590 };

		enterCFGMode();

		writeRegister(COL_GAIN, -1, -1, col_gain[index]);
		writeRegister(BIAS_ADVL_I_H, -1, BIAS_ADVL_I_L, bias_advl_i[index]);
		writeRegister(BIAS_ADVH_I_H, -1, BIAS_ADVH_I_L, bias_advh_i[index]);
		writeRegister(BIAS_ADVCL_I_H, -1, BIAS_ADVCL_I_L, bias_advcl_i[index]);
		writeRegister(BIAS_ADVCH_I_H, -1, BIAS_ADVCH_I_L, bias_advch_i[index]);

		writeRegister(BIAS_VCM_I_H, -1, BIAS_VCM_I_L, bias_vcm_i[index]);

		enterStartMode();
	}
	else
	{
		int index = value - 1;
		if (index < 0)
			index = 0;
		if (index > 3)
			index = 3;

		m_uiISOLevel = index + 1;
		m_pDataProcessor->setISOLevel(m_uiISOLevel);

		int col_gain[4] = { 1, 1, 1, 1 };
		int bias_advl_i[4] = { 428, 384, 336, 284 };
		int bias_advh_i[4] = { 636, 680, 728, 780 };
		int bias_advcl_i[4] = { 506, 495, 483, 470 };
		int bias_advch_i[4] = { 558, 569, 581, 594 };
		int bias_vcm_i[4] = { 532, 532, 532, 532 };

		enterCFGMode();

		writeRegister(COL_GAIN, -1, -1, col_gain[index]);
		writeRegister(BIAS_ADVL_I_H, -1, BIAS_ADVL_I_L, bias_advl_i[index]);
		writeRegister(BIAS_ADVH_I_H, -1, BIAS_ADVH_I_L, bias_advh_i[index]);
		writeRegister(BIAS_ADVCL_I_H, -1, BIAS_ADVCL_I_L, bias_advcl_i[index]);
		writeRegister(BIAS_ADVCH_I_H, -1, BIAS_ADVCH_I_L, bias_advch_i[index]);

		writeRegister(42, -1, 43, bias_vcm_i[index]);

		enterStartMode();
	}
}

/*
*  @function :  getISOLevel
*  @brief    :	get the ISO level of the sensor
*  @input    :
*  @output   :
*  @return   :	the ISO level of the sensor
*/
uint32_t CeleX5::getISOLevel()
{
	return m_uiISOLevel;
}

/*
*  @function :  getISOLevelCount
*  @brief    :	get the ISO count of the sensor
*  @input    :
*  @output   :
*  @return   :	ISO count, 6 for sensor with jumper wire, 4 for sensor without jumper wire
*/
uint32_t CeleX5::getISOLevelCount()
{
	return m_uiISOLevelCount;
}

/*
*  @function :  getFullPicFrameTime
*  @brief    :	get the frame time of full pic
*  @input    :
*  @output   :
*  @return   :	frame time(related to clock rate)
*/
uint32_t CeleX5::getFullPicFrameTime()
{
	return 1000 / m_uiClockRate;
}

/*
*  @function :  setEventFrameTime
*  @brief    :	set the event frame time in Event mode
*  @input    :	value : the frame time of Event Mode, unit is ms
*  @output   :
*  @return   :
*/
void CeleX5::setEventFrameTime(uint32_t value)
{
	m_pDataProcessor->setEventFrameTime(value, m_uiClockRate);
}

/*
*  @function :  getEventFrameTime
*  @brief    :	get the event frame time in Event mode
*  @input    :
*  @output   :
*  @return   :	the frame time of Event Mode, unit is ms
*/
uint32_t CeleX5::getEventFrameTime()
{
	return m_pDataProcessor->getEventFrameTime();
}

/*
*  @function :  setOpticalFlowFrameTime
*  @brief    :	set the frame time in optical flow  mode
*  @input    :	value : the frame time of optical flow Mode, unit is ms
*  @output   :
*  @return   :
*/
void CeleX5::setOpticalFlowFrameTime(uint32_t value)
{
	if (value <= 10 || value >= 180)
	{
		std::cout << __FUNCTION__ << ": value is out of range!" << std::endl;
		return;
	}
	m_uiOpticalFlowFrameTime = value;
	//
	enterCFGMode();
	wireIn(PLL_FOUT3_POST_DIV, (value - 10) * 3 / 2, 0xFF);
	enterStartMode();
}

/*
*  @function :  getOpticalFlowFrameTime
*  @brief    :	get the frame time in optical flow mode
*  @input    :
*  @output   :
*  @return   :	the frame time of optical flow Mode, unit is ms
*/
uint32_t CeleX5::getOpticalFlowFrameTime()
{
	return m_uiOpticalFlowFrameTime;
}

void CeleX5::setEventCountSliceNum(uint32_t value)
{
	m_pDataProcessor->setEventCountSliceNum(value);
}

uint32_t CeleX5::getEventCountSliceNum()
{
	return m_pDataProcessor->getEventCountSliceNum();
}

/*
*  @function :  setEventDuration
*  @brief    :	set the duration of event mode (Mode_A/B/C) when sensor operates in loop mode
*  @input    :	value : the time duration of the Event Mode
*  @output   :
*  @return   :
*/
void CeleX5::setEventDuration(uint32_t value)
{
	enterCFGMode();

	value = value * 1000 / 655;

	uint32_t valueH = value >> 8;
	uint32_t valueL = 0xFF & value;

	wireIn(EVENT_DURATION_H, valueH, 0xFF);
	wireIn(EVENT_DURATION_L, valueL, 0xFF);

	enterStartMode();
}

/*
*  @function :  setPictureNumber
*  @brief    :	set the mumber of pictures
*  @input    :	num : the picture number
*				mode :  the mode need to be set the picture number
*  @output   :
*  @return   :
*/
void CeleX5::setPictureNumber(uint32_t num, CeleX5Mode mode)
{
	enterCFGMode();

	if (Full_Picture_Mode == mode)
		wireIn(PICTURE_NUMBER_1, num, 0xFF);
	else if (Optical_Flow_Mode == mode)
		wireIn(PICTURE_NUMBER_2, num, 0xFF);
	else if (Multi_Read_Optical_Flow_Mode == mode)
		wireIn(PICTURE_NUMBER_4, num, 0xFF);

	enterStartMode();
}

/*
*  @function :  reset
*  @brief    :	reset the sensor and clear the data in the FIFO buffer
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::reset()
{
	if (m_emDeviceType == CeleX5::CeleX5_MIPI)
	{
		enterCFGMode();
		clearData();
		enterStartMode();
	}
}

/*
*  @function :  pauseSensor
*  @brief    :	pause the sensor
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::pauseSensor()
{
	enterCFGMode();
}

/*
*  @function :  restartSensor
*  @brief    :	restart the sensor
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::restartSensor()
{
	enterStartMode();
}

/*
*  @function :  stopSensor
*  @brief    :	stop the sensor and do hadware reset
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::stopSensor()
{
	//hard_reset: bit[3:2], als_enable: bit[1]
	m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 10); //1010
}

/*
*  @function :  getSerialNumber
*  @brief    :	get the serial number of sensor
*  @input    :
*  @output   :
*  @return   :
*/
std::string CeleX5::getSerialNumber()
{
	//cout << "------------- Serial Number: " << m_pCeleDriver->getSerialNumber() << endl;
	return m_pCeleDriver->getSerialNumber();
}

/*
*  @function :  getFirmwareVersion
*  @brief    :	get the firmware version of sensor
*  @input    :
*  @output   :
*  @return   :
*/
std::string CeleX5::getFirmwareVersion()
{
	//cout << "------------- Firmware Version: " << m_pCeleDriver->getFirmwareVersion() << endl;
	return m_pCeleDriver->getFirmwareVersion();
}

/*
*  @function :  getFirmwareDate
*  @brief    :	get the firmware data of sensor
*  @input    :
*  @output   :
*  @return   :
*/
std::string CeleX5::getFirmwareDate()
{
	//cout << "------------- Firmware Date: " << m_pCeleDriver->getFirmwareDate() << endl;
	return m_pCeleDriver->getFirmwareDate();
}

/*
*  @function :  setEventShowMethod
*  @brief    :	set the method to show the event data
*  @input    :	type : the type for creating frame
*				value : the value related to the type
*  @output   :
*  @return   :
*/
void CeleX5::setEventShowMethod(EventShowType type, int value)
{
	m_pDataProcessor->setEventShowMethod(type, value);
	if (type == EventShowByTime)
		setEventFrameTime(value);
}

/*
*  @function :  getEventShowMethod
*  @brief    :	get current event show method
*  @input    :
*  @output   :
*  @return   :
*/
EventShowType CeleX5::getEventShowMethod()
{
	return m_pDataProcessor->getEventShowMethod();
}

/*
*  @function :  setRotateType
*  @brief    :	set rotate type
*  @input    :	type : rotate parameter
*					   1 for up and down flip, 2 for left and right flip, 3 for up and down left and right flip
*  @output   :
*  @return   :
*/
void CeleX5::setRotateType(int type)
{
	//cout << __FUNCTION__ << ": type = " << type << endl;
	m_iRotateType += type;
	m_pDataProcessor->setRotateType(m_iRotateType);
}

/*
*  @function :  getRotateType
*  @brief    :	get current rotate type
*  @input    :
*  @output   :
*  @return   :
*/
int CeleX5::getRotateType()
{
	return m_iRotateType;
}

/*
*  @function :  setEventCountStepSize
*  @brief    :	set the step size of event count picture
*  @input    :	size : the step size
*  @output   :
*  @return   :
*/
void CeleX5::setEventCountStepSize(uint32_t size)
{
	m_pDataProcessor->setEventCountStep(size);
}

/*
*  @function :  getEventCountStepSize
*  @brief    :	get the step size of event count picture
*  @input    :
*  @output   :
*  @return   :
*/
uint32_t CeleX5::getEventCountStepSize()
{
	return m_pDataProcessor->getEventCountStep();
}

/*
*  @function :  setRowDisabled
*  @brief    :	modify the resolution of the image output by CeleX-5 Sensor
*  @input    :	rowMask : bit parameters for the specified rows
*  @output   :
*  @return   :
*/
void CeleX5::setRowDisabled(uint8_t rowMask)
{
	enterCFGMode();
	wireIn(ROW_ENABLE, rowMask, 0xFF);
	enterStartMode();
}

/*
*  @function :  setShowImagesEnabled
*  @brief    :	enable or disable image show
*  @input    :	enable : enable or disable
*  @output   :
*  @return   :
*/
void CeleX5::setShowImagesEnabled(bool enable)
{
	m_bShowImagesEnabled = enable;
	m_pDataProcessThread->setShowImagesEnabled(enable);
}

/*
*  @function :  setEventDataFormat
*  @brief    :	set the event data format to be used
*  @input    :	format : the event data format 0: format 0, 1: format 1, 2: format 2
*  @output   :
*  @return   :
*/
void CeleX5::setEventDataFormat(int format)
{
	m_iEventDataFormat = format;
	wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
	m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);
}

/*
*  @function :  getEventDataFormat
*  @brief    :	get the event data format have been used
*  @input    :
*  @output   :
*  @return   :  the event data format to be used
*/
int CeleX5::getEventDataFormat()
{
	return m_iEventDataFormat;
}

/*
*  @function :  setEventFrameStartPos
*  @brief    :	set the start pos of the event data in the loop mode
*  @input    :	value : start position
*  @output   :
*  @return   :
*/
void CeleX5::setEventFrameStartPos(uint32_t value)
{
	m_pDataProcessor->setEventFrameStartPos(value);
}

/*
*  @function :  setAntiFlashlightEnabled
*  @brief    :	enable or disable the flicker
*  @input    :	enable : enable select: 1:enable / 0:disable
*  @output   :
*  @return   :
*/
void CeleX5::setAntiFlashlightEnabled(bool enabled)
{
	enterCFGMode();
	if (enabled)
		writeRegister(FLICKER_DETECT_EN, -1, -1, 1);
	else
		writeRegister(FLICKER_DETECT_EN, -1, -1, 0);
	enterStartMode();
}

/*
*  @function :  setAutoISPEnabled
*  @brief    :	enable or disable the ISP function
*  @input    :	enable : enable select: 1:enable / 0:disable
*  @output   :
*  @return   :
*/
void CeleX5::setAutoISPEnabled(bool enable)
{
	if (enable)
	{
		enterCFGMode();

		if (!m_bAutoISPFrofileLoaded)
		{
			//--------------- for auto isp --------------- 
			wireIn(AUTOISP_PROFILE_ADDR, 3, 0xFF); //AUTOISP_PROFILE_ADDR
			writeCSRDefaults("Sensor_Core_Parameters"); //Load Sensor Core Parameters
			writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_arrayBrightness[3]);

			wireIn(AUTOISP_PROFILE_ADDR, 2, 0xFF); //AUTOISP_PROFILE_ADDR
			writeCSRDefaults("Sensor_Core_Parameters"); //Load Sensor Core Parameters
			writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_arrayBrightness[2]);

			wireIn(AUTOISP_PROFILE_ADDR, 1, 0xFF); //AUTOISP_PROFILE_ADDR
			writeCSRDefaults("Sensor_Core_Parameters"); //Load Sensor Core Parameters
			writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_arrayBrightness[1]);

			//wireIn(AUTOISP_BRT_EN, 0, 0xFF); //AUTOISP_BRT_EN, disable auto ISP
			//wireIn(AUTOISP_TEM_EN, 0, 0xFF); //AUTOISP_TEM_EN
			//wireIn(AUTOISP_TRIGGER, 0, 0xFF); //AUTOISP_TRIGGER

			writeRegister(225, -1, 224, m_uiAutoISPRefreshTime); //AUTOISP_REFRESH_TIME

			writeRegister(AUTOISP_BRT_THRES1_H, -1, AUTOISP_BRT_THRES1_L, m_arrayISPThreshold[0]); //AUTOISP_BRT_THRES1
			writeRegister(AUTOISP_BRT_THRES2_H, -1, AUTOISP_BRT_THRES2_L, m_arrayISPThreshold[1]); //AUTOISP_BRT_THRES2
			writeRegister(AUTOISP_BRT_THRES3_H, -1, AUTOISP_BRT_THRES3_L, m_arrayISPThreshold[2]); //AUTOISP_BRT_THRES3

			writeRegister(AUTOISP_BRT_VALUE_H, -1, AUTOISP_BRT_VALUE_L, 1500); //AUTOISP_BRT_VALUE

			m_bAutoISPFrofileLoaded = true;

		}
		wireIn(AUTOISP_BRT_EN, 1, 0xFF); //AUTOISP_BRT_EN, enable auto ISP
		if (isLoopModeEnabled())
			wireIn(AUTOISP_TRIGGER, 1, 0xFF); //AUTOISP_TRIGGER
		else
			wireIn(AUTOISP_TRIGGER, 0, 0xFF); //AUTOISP_TRIGGER

		wireIn(AUTOISP_PROFILE_ADDR, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
		writeRegister(AUTOISP_BRT_VALUE_H, -1, AUTOISP_BRT_VALUE_L, 1500); //AUTOISP_BRT_VALUE, Set initial brightness value 1500
		writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_uiBrightness); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0

		enterStartMode();

		setALSEnabled(true);
		m_bAutoISPEnabled = true;
	}
	else
	{
		setALSEnabled(false); //Disable ALS read and write
		m_bAutoISPEnabled = false;

		enterCFGMode();

		//Disable brightness adjustment (auto isp), always load sensor core parameters from profile0
		wireIn(AUTOISP_BRT_EN, 0, 0xFF); //AUTOISP_BRT_EN, disable auto ISP
		wireIn(AUTOISP_PROFILE_ADDR, 0, 0xFF); //AUTOISP_PROFILE_ADDR, Write core parameters to profile0
		writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_uiBrightness); //BIAS_BRT_I, Override the brightness value in profile0, avoid conflict with AUTOISP profile0

		enterStartMode();
	}
}

/*
*  @function :  isAutoISPEnabled
*  @brief    :	get the auto ISP state
*  @input    :
*  @output   :
*  @return   :	bool : true for enabled, false for diabled
*/
bool CeleX5::isAutoISPEnabled()
{
	return m_bAutoISPEnabled;
}

/*
*  @function :  setALSEnabled
*  @brief    :	enable or disable the ALS function
*  @input    :	enable : enable select: 1:enable / 0:disable
*  @output   :
*  @return   :
*/
void CeleX5::setALSEnabled(bool enable)
{
	//als: bit[1] = 1 menas disabled; bit[1] = 0 means enabled
	if (enable)
	{
		if (isIMUModuleEnabled())
			m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0100);
		else
			m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0000);
	}
	else
	{
		if (isIMUModuleEnabled())
			m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0102);
		else
			m_pCeleDriver->i2cSet(VIRTUAL_USB_ADDR, 0x0002);
	}
	m_bALSEnabled = enable;
}

/*
*  @function :  isALSEnabled
*  @brief    :	get the ALS state
*  @input    :
*  @output   :
*  @return   :	bool : true for enabled, false for diabled
*/
bool CeleX5::isALSEnabled()
{
	return m_bALSEnabled;
}

/*
*  @function :  setISPThreshold
*  @brief    :	set the threshold of ISP
*  @input    :	value : threshold
*				num : there are 3 sets of threshold, num is the index of sets
*  @output   :
*  @return   :
*/
void CeleX5::setISPThreshold(uint32_t value, int num)
{
	m_arrayISPThreshold[num - 1] = value;
	if (num == 1)
		writeRegister(AUTOISP_BRT_THRES1_H, -1, AUTOISP_BRT_THRES1_L, m_arrayISPThreshold[0]); //AUTOISP_BRT_THRES1
	else if (num == 2)
		writeRegister(AUTOISP_BRT_THRES2_H, -1, AUTOISP_BRT_THRES2_L, m_arrayISPThreshold[1]); //AUTOISP_BRT_THRES2
	else if (num == 3)
		writeRegister(AUTOISP_BRT_THRES3_H, -1, AUTOISP_BRT_THRES3_L, m_arrayISPThreshold[2]); //AUTOISP_BRT_THRES3
}

/*
*  @function :  setISPBrightness
*  @brief    :	set the brightness of ISP
*  @input    :	value : brightness
*				num : there are 3 sets of brightness, num is the index of sets
*  @output   :
*  @return   :
*/
void CeleX5::setISPBrightness(uint32_t value, int num)
{
	m_arrayBrightness[num - 1] = value;
	wireIn(AUTOISP_PROFILE_ADDR, num - 1, 0xFF); //AUTOISP_PROFILE_ADDR
	writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_arrayBrightness[num - 1]);
}

/*
*  @function :  startRecording
*  @brief    :	start recording the raw data of the sensor and save it as a bin file
*  @input    :	filePath : the directory path to save the bin file
*  @output   :
*  @return   :
*/
void CeleX5::startRecording(const std::string& filePath)
{
	m_pDataRecorder->startRecording(filePath);
	m_pDataProcessThread->setRecordState(true);
}

/*
*  @function :  stopRecording
*  @brief    :	stop recording the raw data of the sensor
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::stopRecording()
{
	if (CeleX5::CeleX5_MIPI == m_emDeviceType)
	{
		BinFileAttributes header;
		if (isLoopModeEnabled())
		{
			header.dataType = 3;
			header.loopAMode = m_pDataProcessor->getSensorLoopMode(1);
			header.loopBMode = m_pDataProcessor->getSensorLoopMode(2);
			header.loopCMode = m_pDataProcessor->getSensorLoopMode(3);
		}
		else
		{
			header.dataType = 2;
			header.loopAMode = m_pDataProcessor->getSensorFixedMode();
			header.loopBMode = 255;
			header.loopCMode = 255;
		}
		header.eventDataFormat = m_iEventDataFormat;
		m_pDataRecorder->stopRecording(&header);
	}
	m_pDataProcessThread->setRecordState(false);
}

/*
*  @function :  openBinFile
*  @brief    :	open the bin file in the user-specified directory
*  @input    :	filePath : the directory path and name of the bin file to be played
*  @output   :
*  @return   :	bool value whether the bin file is opened
*/
bool CeleX5::openBinFile(const std::string& filePath)
{
	m_uiPackageCount = 0;
	m_bFirstReadFinished = false;
	m_vecPackagePos.clear();
	if (m_ifstreamPlayback.is_open())
	{
		m_ifstreamPlayback.close();
	}
	m_ifstreamPlayback.open(filePath.c_str(), std::ios::binary);
	if (!m_ifstreamPlayback.good())
	{
		std::cout << "Can't Open File: " << filePath.c_str() << std::endl;
		return false;
	}
	m_pDataProcessThread->clearData();
	m_pDataProcessThread->setIsPlayback(true);
	m_pDataProcessThread->setPlaybackState(Playing);
	m_pDataProcessor->resetTimestamp();
	// read header
	m_ifstreamPlayback.read((char*)&m_stBinFileHeader, sizeof(BinFileAttributes));
	std::cout << "data_type = " << (int)m_stBinFileHeader.dataType
		<< ", loopA_mode = " << (int)m_stBinFileHeader.loopAMode
		<< ", loopB_mode = " << (int)m_stBinFileHeader.loopBMode
		<< ", loopC_mode = " << (int)m_stBinFileHeader.loopCMode
		<< ", event_data_format = " << (int)m_stBinFileHeader.eventDataFormat
		<< ", hour = " << (int)m_stBinFileHeader.hour
		<< ", minute = " << (int)m_stBinFileHeader.minute
		<< ", second = " << (int)m_stBinFileHeader.second
		<< ", package_count = " << (int)m_stBinFileHeader.packageCount << std::endl;

	m_uiTotalPackageCount = m_stBinFileHeader.packageCount;
	m_pDataProcessThread->start();
	setEventDataFormat(m_stBinFileHeader.eventDataFormat);
	if (m_stBinFileHeader.dataType == 1 || m_stBinFileHeader.dataType == 3)
	{
		m_bLoopModeEnabled = true;
		m_pDataProcessor->setLoopModeEnabled(true);
		m_pDataProcessor->setSensorLoopMode(CeleX5::CeleX5Mode(m_stBinFileHeader.loopAMode), 1);
		m_pDataProcessor->setSensorLoopMode(CeleX5::CeleX5Mode(m_stBinFileHeader.loopBMode), 2);
		m_pDataProcessor->setSensorLoopMode(CeleX5::CeleX5Mode(m_stBinFileHeader.loopCMode), 3);
	}
	else
	{
		m_bLoopModeEnabled = false;
		m_pDataProcessor->setSensorFixedMode(CeleX5::CeleX5Mode(m_stBinFileHeader.loopAMode));
	}
	return true;
}

/*
*  @function :  readBinFileData
*  @brief    :	read data from the opened bin file
*  @input    :
*  @output   :
*  @return   :	bool value whether the bin is read over
*/
bool CeleX5::readBinFileData()
{
	//cout << __FUNCTION__ << endl;
	if (m_pDataProcessThread->queueSize() > 10)
	{
		return false;
	}
	bool eof = false;

	uint64_t ifReadPos = m_ifstreamPlayback.tellg();

	uint32_t lenToRead = 0;
	m_ifstreamPlayback.read((char*)&lenToRead, 4);
	//cout << "lenToRead = " << lenToRead << endl;

	if (NULL == m_pDataToRead)
		m_pDataToRead = new uint8_t[2048000];
	m_ifstreamPlayback.read((char*)m_pDataToRead, lenToRead);
	//
	int dataLen = m_ifstreamPlayback.gcount();
	if (dataLen > 0)
	{
		m_uiPackageCount++;
		if ((0x02 & m_stBinFileHeader.dataType) == 0x02) //has IMU data
		{
			time_t timeStamp;
			m_ifstreamPlayback.read((char*)&timeStamp, 8);
			//cout << "timeStamp = " << timeStamp << endl;
			//m_pDataProcessThread->addData(m_pDataToRead, dataLen, timeStamp);

			int imuSize = 0;
			IMURawData imuRawData;
			std::vector<IMURawData> vecIMUData;
			m_ifstreamPlayback.read((char*)&imuSize, 4);
			if (imuSize > 0)
			{
				//cout << "imu size = " << imuSize << endl;
				for (int i = 0; i < imuSize; i++)
				{
					m_ifstreamPlayback.read((char*)&imuRawData, sizeof(IMURawData));
					//cout << "imuRawData.time_stamp: " << imuRawData.time_stamp << endl;
					vecIMUData.push_back(imuRawData);
				}
				//m_pDataProcessThread->addIMUData(vecIMUData);
			}
			m_pDataProcessThread->addData(m_pDataToRead, dataLen, vecIMUData, timeStamp);
		}
		else
		{
			m_pDataProcessThread->addData(m_pDataToRead, dataLen, 0);
		}
		if (!m_bFirstReadFinished)
		{
			m_vecPackagePos.push_back(ifReadPos);
			//cout << "--------------- " << m_vecPackagePos[m_uiPackageCount-1] << endl;
		}
		//cout << "package_count = " << m_uiPackageCount << endl;
	}
	if (m_ifstreamPlayback.eof())
	{
		eof = true;
		m_bFirstReadFinished = true;
		//m_ifstreamPlayback.close();
		setPlaybackState(BinReadFinished);
		std::cout << "Read Playback file Finished!" << std::endl;

		m_uiTotalPackageCount = m_uiPackageCount;
	}
	return eof;
}

/*
*  @function :  getTotalPackageCount
*  @brief    :	get total package count of data
*  @input    :
*  @output   :
*  @return   :	package count
*/
uint32_t CeleX5::getTotalPackageCount()
{
	return m_uiTotalPackageCount;
}

/*
*  @function :  getCurrentPackageNo
*  @brief    :	get current package number of data
*  @input    :
*  @output   :
*  @return   :	package number
*/
uint32_t CeleX5::getCurrentPackageNo()
{
	//cout << "getCurrentPackageNo: " << m_pDataProcessThread->getPackageNo() << endl;
	return m_pDataProcessThread->getPackageNo();
}

/*
*  @function :  setCurrentPackageNo
*  @brief    :	set current package number of data
*  @input    :	value : package number
*  @output   :
*  @return   :
*/
void CeleX5::setCurrentPackageNo(uint32_t value)
{
	setPlaybackState(Replay);
	m_uiPackageCount = value;
	m_ifstreamPlayback.clear();
	m_ifstreamPlayback.seekg(m_vecPackagePos[value], std::ios::beg);
	m_pDataProcessThread->setPackageNo(value);
}

/*
*  @function :  getBinFileAttributes
*  @brief    :	get the attributes of the bin file
*  @input    :
*  @output   :
*  @return   :	a structure of the file attributes
*/
CeleX5::BinFileAttributes CeleX5::getBinFileAttributes()
{
	return m_stBinFileHeader;
}

/*
*  @function :  replay
*  @brief    :	replay the bin file
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::replay()
{
	setPlaybackState(Replay);
	m_uiPackageCount = 0;
	m_ifstreamPlayback.clear();
	m_ifstreamPlayback.seekg(sizeof(BinFileAttributes), std::ios::beg);
	m_pDataProcessThread->setPackageNo(0);
}

/*
*  @function :  play
*  @brief    :	play the bin file
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::play()
{
	m_pDataProcessThread->resume();
}

/*
*  @function :  pause
*  @brief    :	pause the bin file
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::pause()
{
	m_pDataProcessThread->suspend();
}

/*
*  @function :  getPlaybackState
*  @brief    :	get the state of playback
*  @input    :
*  @output   :
*  @return   :	current play state
*/
PlaybackState CeleX5::getPlaybackState()
{
	return m_pDataProcessThread->getPlaybackState();
}

/*
*  @function :  setPlaybackState
*  @brief    :	set the state of playback
*  @input    :	state : playback state
*  @output   :
*  @return   :	
*/
void CeleX5::setPlaybackState(PlaybackState state)
{
	m_pDataProcessThread->setPlaybackState(state);
}

/*
*  @function :  setIsPlayBack
*  @brief    :	set the state of playback
*  @input    :	state : true for is playback, false for not playing
*  @output   :
*  @return   :
*/
void CeleX5::setIsPlayBack(bool state)
{
	m_pDataProcessThread->setIsPlayback(state);
	if (!state) //stop playback
	{
		if (m_ifstreamPlayback.is_open())
		{
			m_ifstreamPlayback.close();
		}
	}
}

void CeleX5::saveBinFile(std::string srcPath, int startPackageCount, int endPackageCount)
{
	if (m_ifstreamPlayback.is_open())
	{
		m_ifstreamPlayback.close();
	}
	m_ifstreamPlayback.open(srcPath.c_str(), std::ios::binary);
	if (!m_ifstreamPlayback.good())
	{
		std::cout << "Can't Open File: " << srcPath.c_str() << std::endl;
	}

	if (startPackageCount > endPackageCount)
		std::cout << "startPackageCount > endPackageCount" << std::endl;
	if (startPackageCount >= m_vecPackagePos.size() - 1)
		std::cout << "startPackageCount >= m_vecPackagePos.size() - 1" << std::endl;
	if (endPackageCount >= m_vecPackagePos.size() - 1)
		std::cout << "endPackageCount >= m_vecPackagePos.size() - 1" << std::endl;

	std::cout << "m_vecPackagePos[startPackageCount] = " << m_vecPackagePos[startPackageCount] << std::endl;
	std::cout << "m_vecPackagePos[endPackageCount] = " << m_vecPackagePos[endPackageCount] << std::endl;

	m_ifstreamPlayback.seekg(m_vecPackagePos[startPackageCount], std::ios::beg);

	std::ofstream ofBinClip;
	std::string objPath = srcPath.insert(srcPath.size() - 4, "_clip");
	ofBinClip.open(objPath, std::ios::binary);
	std::cout << "objPath = " << objPath << std::endl;
	ofBinClip.write((char*)&m_stBinFileHeader, sizeof(BinFileAttributes));

	for (int i = 0; i < endPackageCount - startPackageCount; i++)
	{
		int len = m_vecPackagePos[startPackageCount + i + 1] - m_vecPackagePos[startPackageCount + i];
		
		//m_ifstreamPlayback.read(buffer, len);
		//cout << "len = " << len << endl;
		uint32_t lenToRead = 0;
		m_ifstreamPlayback.read((char*)&lenToRead, 4);
		//cout << "lenToRead = " << lenToRead << endl;
		ofBinClip.write((char*)&lenToRead, 4);

		char* buffer = new char[lenToRead];
		m_ifstreamPlayback.read(buffer, lenToRead);
		ofBinClip.write(buffer, lenToRead);

		if ((0x02 & m_stBinFileHeader.dataType) == 0x02) //has IMU data
		{
			time_t timeStamp;
			m_ifstreamPlayback.read((char*)&timeStamp, 8);
			//cout << "timestamp = " << timeStamp << endl;
			ofBinClip.write((char*)&timeStamp, 8);

			int imuSize = 0;
			IMURawData imuRawData;
			m_ifstreamPlayback.read((char*)&imuSize, 4);
			ofBinClip.write((char*)&imuSize, 4);
			//cout << "imu size = " << imuSize << endl;
			if (imuSize > 0)
			{
				for (int i = 0; i < imuSize; i++)
				{
					m_ifstreamPlayback.read((char*)&imuRawData, sizeof(IMURawData));
					//cout << "imuRawData.time_stamp: " << imuRawData.time_stamp << endl;
					ofBinClip.write((char*)&imuRawData, sizeof(IMURawData));
				}
			}
		}
		delete[] buffer;
	}
	ofBinClip.close();
	std::cout << "------------ Save clip bin file successfully! ------------" << std::endl;
}

/*
*  @function :  getSensorDataServer
*  @brief    :	get CX5SensorDataServer pointer
*  @input    :
*  @output   :
*  @return   :	CX5SensorDataServer pointer
*/
CX5SensorDataServer* CeleX5::getSensorDataServer()
{
	return m_pDataProcessor->getSensorDataServer();
}

/*
*  @function :  getDeviceType
*  @brief    :	get current device type of sensor
*  @input    :
*  @output   :
*  @return   :	device type
*/
CeleX5::DeviceType CeleX5::getDeviceType()
{
	return m_emDeviceType;
}

/*
*  @function :  getFullFrameFPS
*  @brief    :	get FPS of full frame buffer
*  @input    :
*  @output   :
*  @return   :	frame FPS of full pic
*/
uint32_t CeleX5::getFullFrameFPS()
{
	return m_uiPackageCountPS;
}

/*
*  @function :  getEventRate
*  @brief    :	get event rate(events per second)
*  @input    :
*  @output   :
*  @return   :	event rate 
*/
uint32_t CeleX5::getEventRate()
{
	return m_pDataProcessor->getEventRate();
}

uint32_t CeleX5::getEventRatePerFrame()
{
	return m_pDataProcessor->getEventRatePerFrame();;
}

/*
*  @function :  getALSValue
*  @brief    :	get ALS value
*  @input    :
*  @output   :
*  @return   :	ALS value
*/
int CeleX5::getALSValue()
{
	return m_pCeleDriver->getALSValue();
}

/*
*  @function :  getCeleX5Cfg
*  @brief    :	get the parameters in the config file
*  @input    :
*  @output   :
*  @return   :	key-value type parameters in the config file
*/
std::map<std::string, std::vector<CeleX5::CfgInfo>> CeleX5::getCeleX5Cfg()
{
	std::map<std::string, std::vector<WireinCommand*>> mapCfg = m_pCeleX5CfgMgr->getCeleX5Cfg();
	//
	std::map<std::string, std::vector<CeleX5::CfgInfo>> mapCfg1;
	for (auto itr = mapCfg.begin(); itr != mapCfg.end(); itr++)
	{
		//cout << "CeleX5::getCeleX5Cfg: " << itr->first << endl;
		std::vector<WireinCommand*> pCmdList = itr->second;
		std::vector<CeleX5::CfgInfo> vecCfg;
		for (auto itr1 = pCmdList.begin(); itr1 != pCmdList.end(); itr1++)
		{
			WireinCommand* pCmd = (WireinCommand*)(*itr1);
			//cout << "----- Register Name: " << pCmd->getName() << endl;
			CeleX5::CfgInfo cfgInfo;
			cfgInfo.name = pCmd->getName();
			cfgInfo.min = pCmd->getMinValue();
			cfgInfo.max = pCmd->getMaxValue();
			cfgInfo.value = pCmd->getValue();
			cfgInfo.highAddr = pCmd->getHighAddr();
			cfgInfo.middleAddr = pCmd->getMiddleAddr();
			cfgInfo.lowAddr = pCmd->getLowAddr();
			vecCfg.push_back(cfgInfo);
		}
		mapCfg1[itr->first] = vecCfg;
	}
	return mapCfg1;
}

/*
*  @function :  getCeleX5CfgModified
*  @brief    :	get the modified parameters in the config file
*  @input    :
*  @output   :
*  @return   :	key-value type parameters in the modified config file
*/
std::map<std::string, std::vector<CeleX5::CfgInfo>> CeleX5::getCeleX5CfgModified()
{
	return m_mapCfgModified;
}

/*
*  @function :  writeRegister
*  @brief    :	write register
*  @input    :	addressH : address at high position
*				addressM : address at middle position
*				addressL : address at low position
*				value : 8-bit address
*  @output   :
*  @return   :
*/
void CeleX5::writeRegister(int16_t addressH, int16_t addressM, int16_t addressL, uint32_t value)
{
	if (addressL == -1)
	{
		wireIn(addressH, value, 0xFF);
	}
	else
	{
		if (addressM == -1)
		{
			uint32_t valueH = value >> 8;
			uint32_t valueL = 0xFF & value;
			wireIn(addressH, valueH, 0xFF);
			wireIn(addressL, valueL, 0xFF);
		}
	}
}

/*
*  @function :  getCfgInfoByName
*  @brief    :	get the config parameters by each register
*  @input    :  csrType : the group type of the registers
*				name : the name of register
*				bDefault : true for default config file, false for modified config file
*  @output   :
*  @return   :	configuration infomations of each register
*/
CeleX5::CfgInfo CeleX5::getCfgInfoByName(const std::string& csrType, const std::string& name, bool bDefault)
{
	std::map<std::string, std::vector<CfgInfo>> mapCfg;
	if (bDefault)
		mapCfg = m_mapCfgDefaults;
	else
		mapCfg = m_mapCfgModified;
	CeleX5::CfgInfo cfgInfo;
	for (auto itr = mapCfg.begin(); itr != mapCfg.end(); itr++)
	{
		std::string tapName = itr->first;
		if (csrType == tapName)
		{
			std::vector<CfgInfo> vecCfg = itr->second;
			for (auto itrCfg = vecCfg.begin(); itrCfg != vecCfg.end(); itrCfg++)
			{
				if ((*itrCfg).name == name)
				{
					cfgInfo = (*itrCfg);
					return cfgInfo;
				}
			}
			break;
		}
	}
	return cfgInfo;
}

/*
*  @function :  writeCSRDefaults
*  @brief    :	write the control system register
*  @input    :  csrType : the group type of the registers
*  @output   :
*  @return   :
*/
void CeleX5::writeCSRDefaults(const std::string& csrType)
{
	std::cout << "CeleX5::writeCSRDefaults: " << csrType << std::endl;
	m_mapCfgModified[csrType] = m_mapCfgDefaults[csrType];
	for (auto itr = m_mapCfgDefaults.begin(); itr != m_mapCfgDefaults.end(); itr++)
	{
		//cout << "group name: " << itr->first << endl;
		std::string tapName = itr->first;
		if (csrType == tapName)
		{
			std::vector<CeleX5::CfgInfo> vecCfg = itr->second;
			for (auto itrCfg = vecCfg.begin(); itrCfg != vecCfg.end(); itrCfg++)
			{
				CeleX5::CfgInfo cfgInfo = (*itrCfg);
				writeRegister(cfgInfo);
			}
			break;
		}
	}
}

/*
*  @function :  modifyCSRParameter
*  @brief    :	modify the control system register parameters
*  @input    :  csrType : the group type of the registers
*				cmdName : command name
*				value : 8-bit address value
*  @output   :
*  @return   :
*/
void CeleX5::modifyCSRParameter(const std::string& csrType, const std::string& cmdName, uint32_t value)
{
	CeleX5::CfgInfo cfgInfo;
	for (auto itr = m_mapCfgModified.begin(); itr != m_mapCfgModified.end(); itr++)
	{
		std::string tapName = itr->first;
		if (csrType.empty())
		{
			std::vector<CfgInfo> vecCfg = itr->second;
			int index = 0;
			for (auto itrCfg = vecCfg.begin(); itrCfg != vecCfg.end(); itrCfg++)
			{
				if ((*itrCfg).name == cmdName)
				{
					cfgInfo = (*itrCfg);
					std::cout << "CeleX5::modifyCSRParameter: Old value = " << cfgInfo.value << std::endl;
					//modify the value in m_pMapCfgModified
					cfgInfo.value = value;
					vecCfg[index] = cfgInfo;
					m_mapCfgModified[tapName] = vecCfg;
					std::cout << "CeleX5::modifyCSRParameter: New value = " << cfgInfo.value << std::endl;
					break;
				}
				index++;
			}
		}
		else
		{
			if (csrType == tapName)
			{
				std::vector<CfgInfo> vecCfg = itr->second;
				int index = 0;
				for (auto itrCfg = vecCfg.begin(); itrCfg != vecCfg.end(); itrCfg++)
				{
					if ((*itrCfg).name == cmdName)
					{
						cfgInfo = (*itrCfg);
						std::cout << "CeleX5::modifyCSRParameter: Old value = " << cfgInfo.value << std::endl;
						//modify the value in m_pMapCfgModified
						cfgInfo.value = value;
						vecCfg[index] = cfgInfo;
						m_mapCfgModified[tapName] = vecCfg;
						std::cout << "CeleX5::modifyCSRParameter: New value = " << cfgInfo.value << std::endl;
						break;
					}
					index++;
				}
				break;
			}
		}
	}
	m_pCeleX5CfgMgr->saveCeleX5XML(m_mapCfgModified);
}

/*
*  @function :  configureSettings
*  @brief    :	configure settings
*  @input    :  type : the type of current device
*  @output   :
*  @return   :	bool : true for configured successfully
*/
bool CeleX5::configureSettings(CeleX5::DeviceType type)
{
	if (CeleX5::CeleX5_MIPI == type)
	{
		setALSEnabled(false);
		//--------------- Step1 ---------------
		wireIn(PADDR_EN, 0, 0xFF); //PADDR_EN

		//--------------- Step2: Load PLL Parameters ---------------
		//Disable PLL
		std::cout << "--- Disable PLL ---" << std::endl;
		wireIn(PLL_PD_B, 0, 0xFF); //PLL_PD_B
		//Load PLL Parameters
		std::cout << std::endl << "--- Load PLL Parameters ---" << std::endl;
		writeCSRDefaults("PLL_Parameters");
		//Enable PLL
		std::cout << "--- Enable PLL ---" << std::endl;
		wireIn(PLL_PD_B, 1, 0xFF); //PLL_PD_B

		//--------------- Step3: Load MIPI Parameters ---------------
		std::cout << std::endl << "--- Disable MIPI ---" << std::endl;
		disableMIPI();

		std::cout << std::endl << "--- Load MIPI Parameters ---" << std::endl;
		writeCSRDefaults("MIPI_Parameters");
		//writeRegister(MIPI_PLL_DIV_N_H, -1, MIPI_PLL_DIV_N_L, 120); //MIPI_PLL_DIV_N

		//Enable MIPI
		std::cout << std::endl << "--- Enable MIPI ---" << std::endl;
		enableMIPI();

		//--------------- Step4: ---------------
		std::cout << std::endl << "--- Enter CFG Mode ---" << std::endl;
		enterCFGMode();

		//----- Load Sensor Core Parameters -----
		wireIn(AUTOISP_PROFILE_ADDR, 0, 0xFF); //AUTOISP_PROFILE_ADDR
		writeCSRDefaults("Sensor_Core_Parameters"); //Load Sensor Core Parameters
		writeRegister(BIAS_BRT_I_H, -1, BIAS_BRT_I_L, m_uiBrightness);

		writeCSRDefaults("Sensor_Operation_Mode_Control_Parameters");

		writeCSRDefaults("Sensor_Data_Transfer_Parameters");
		wireIn(EVENT_PACKET_SELECT, m_iEventDataFormat, 0xFF); //EVENT_PACKET_SELECT
		m_pDataProcessor->setMIPIDataFormat(m_iEventDataFormat);

		std::cout << std::endl << "--- Enter Start Mode ---" << std::endl;
		enterStartMode();

		if (m_pCeleDriver)
			m_pCeleDriver->openStream();
	}
	return true;
}

/*
*  @function :  wireIn
*  @brief    :	write register
*  @input    :  address : address of register
*				value : value of register
*  @output   :
*  @return   :
*/
void CeleX5::wireIn(uint32_t address, uint32_t value, uint32_t mask)
{
	if (CeleX5::CeleX5_MIPI == m_emDeviceType)
	{
		if (m_pCeleDriver)
		{
			if (isAutoISPEnabled())
			{
				setALSEnabled(false);
#ifdef _WIN32
				Sleep(2);
#else
				usleep(1000 * 2);
#endif
			}
			if (m_pCeleDriver->i2cSet(address, value))
			{
				//cout << "CeleX5::wireIn(i2c_set): address = " << address << ", value = " << value << endl;
			}
			if (isAutoISPEnabled())
			{
				setALSEnabled(true);
			}
		}
	}
}

/*
*  @function :  writeRegister
*  @brief    :	write registers
*  @input    :  cfgInfo : configuration infomations of each register
*  @output   :
*  @return   :
*/
void CeleX5::writeRegister(CfgInfo cfgInfo)
{
	if (cfgInfo.lowAddr == -1)
	{
		wireIn(cfgInfo.highAddr, cfgInfo.value, 0xFF);
	}
	else
	{
		if (cfgInfo.middleAddr == -1)
		{
			uint32_t valueH = cfgInfo.value >> 8;
			uint32_t valueL = 0xFF & cfgInfo.value;
			wireIn(cfgInfo.highAddr, valueH, 0xFF);
			wireIn(cfgInfo.lowAddr, valueL, 0xFF);
		}
	}
}

/*
*  @function :  enterCFGMode
*  @brief    :	enter config mode
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enterCFGMode()
{
	wireIn(SOFT_TRIGGER, 0, 0xFF);
	wireIn(SOFT_RESET, 1, 0xFF);
}

/*
*  @function :  enterStartMode
*  @brief    :	enter start mode
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enterStartMode()
{
	wireIn(SOFT_RESET, 0, 0xFF);
	wireIn(SOFT_TRIGGER, 1, 0xFF);
}

/*
*  @function :  disableMIPI
*  @brief    :	disable the MIPI when the registers need to be written
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::disableMIPI()
{
	wireIn(MIPI_NPOWD_PLL, 0, 0xFF);
	wireIn(MIPI_NPOWD_BGR, 0, 0xFF);
	wireIn(MIPI_NRSET_PLL, 0, 0xFF);
	wireIn(MIPI_NPOWD_PHY, 0, 0xFF);
	wireIn(MIPI_NRSET_PHY, 0, 0xFF);
	wireIn(MIPI_NDIS_PHY, 0, 0xFF);
}

/*
*  @function :  enableMIPI
*  @brief    :	enable the MIPI when the registers have been written
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::enableMIPI()
{
	wireIn(MIPI_NPOWD_PLL, 1, 0xFF);
	wireIn(MIPI_NPOWD_BGR, 1, 0xFF);
	wireIn(MIPI_NRSET_PLL, 1, 0xFF);
	wireIn(MIPI_NPOWD_PHY, 1, 0xFF);
	wireIn(MIPI_NRSET_PHY, 1, 0xFF);
	wireIn(MIPI_NDIS_PHY, 1, 0xFF);
}

/*
*  @function :  clearData
*  @brief    :	clear the data buffer
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::clearData()
{
	m_pDataProcessThread->clearData();
	if (CeleX5::CeleX5_MIPI == m_emDeviceType)
	{
		m_pCeleDriver->clearData();
	}
}

int CeleX5::denoisingMaskByEventTime(const cv::Mat& countEventImg, double timelength, cv::Mat& denoiseMaskImg)
{
	if (countEventImg.empty())
	{
		return 0;
	}
	//cv::Mat kern = (cv::Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);//4-neighbor for convolution
	//cv::Mat convimg;
	//countEventImg.convertTo(convimg, CV_32FC1);
	//cv::filter2D(convimg, convimg, convimg.depth(), kern);

	//int timeslicecount = timelength / 800;//time step for density estimation, assigned by experience
	//int thresh = timeslicecount * 3;
	//cv::threshold(convimg, denoiseMaskImg, thresh, 255, CV_THRESH_BINARY);
	//denoiseMaskImg.convertTo(denoiseMaskImg, CV_8UC1);
	//return 1;

	cv::Mat kern = (cv::Mat_<uchar>(3, 3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
	//cv::Mat kern = (cv::Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);
	cv::Mat convimg;
	cv::filter2D(countEventImg, convimg, countEventImg.depth(), kern);

	int timeslicecount = timelength / 30.0;//time step for density estimation, assigned by experience
	int thresh = timeslicecount * 5;
	cv::threshold(convimg, denoiseMaskImg, thresh, 255, CV_THRESH_BINARY);
	denoiseMaskImg.convertTo(denoiseMaskImg, CV_8UC1);
	return 1;
}

/*
*  @function :  saveFullPicRawData
*  @brief    :	save a raw data of full pic(for test)
*  @input    :
*  @output   :
*  @return   :
*/
void CeleX5::saveFullPicRawData()
{
	m_pDataProcessor->saveFullPicRawData();
}

/*
*  @function :  calDirectionAndSpeedEx
*  @brief    :	caculate the speed buffer and direction buffer of the optical buffer
*  @input    :	pBuffer : the original optical flow buffer
*				speedBuffer : the speed buffer of optical flow buffer
*				dirBuffer : the direction buffer of optical flow buffer
*  @output   :
*  @return   :	
*/
void CeleX5::calDirectionAndSpeedEx(cv::Mat pBuffer, cv::Mat &speedBuffer, cv::Mat &dirBuffer)
{
	cv::Mat pBufferBlurred;
	float neighborDist = 4;
	int timeThreshold = 1000;
	int minTime = 1;
	int maxTime = 200; //set interval that is counted for OF estimation: [minTime, maxTime]
	float velocityThresMin = 0.05; // unit: pixel/time unit (1 frame = 255 time units)
	float velocityThresMax = 0.5; //////////////////////////////////////// An important param.
	int u_min_timestamp, v_min_timestamp;
	int timeStampLeft, timeStampRight, timeStampUp, timeStampDown, deltaTimeLeft, deltaTimeRight, deltaTimeUp, deltaTimeDown;

	cv::medianBlur(pBuffer, pBufferBlurred, 3);
	pBuffer = pBufferBlurred;

	for (int i = 0; i < CELEX5_ROW; i = i + 2)
	{
		for (int j = 0; j < CELEX5_COL; j = j + 2)
		{
			// float Gx = 0, Gy = 0;
			double u = 0, v = 0; // u = 1/Gx, v = 1/Gy
			int timeStampCenter = pBuffer.at<uchar>(i, j);
			if (timeStampCenter < minTime || timeStampCenter>maxTime)
			{
				// cout<<timeStampCenter<<endl;
			}
			else
			{
				if (j <= neighborDist - 1 || j >= CELEX5_COL - neighborDist)
					u = 0;
				else
				{
					timeStampLeft = pBuffer.at<uchar>(i, j - neighborDist);
					timeStampRight = pBuffer.at<uchar>(i, j + neighborDist);
					deltaTimeLeft = timeStampCenter - timeStampLeft;
					deltaTimeRight = timeStampCenter - timeStampRight;

					if (pBuffer.at<uchar>(i, j + neighborDist) - pBuffer.at<uchar>(i, j - neighborDist) != 0)
					{
						u = (2 * neighborDist) / (pBuffer.at<uchar>(i, j + neighborDist) - pBuffer.at<uchar>(i, j - neighborDist));
						u_min_timestamp = min(pBuffer.at<uchar>(i, j + neighborDist), pBuffer.at<uchar>(i, j - neighborDist));
					}
				}

				if (i <= neighborDist - 1 || i >= CELEX5_ROW - neighborDist)
					v = 0;
				else
				{
					timeStampUp = pBuffer.at<uchar>(i - neighborDist, j);
					timeStampDown = pBuffer.at<uchar>(i + neighborDist, j);
					deltaTimeUp = timeStampCenter - timeStampUp;
					deltaTimeDown = timeStampCenter - timeStampDown;

					if (pBuffer.at<uchar>(i + neighborDist, j) - pBuffer.at<uchar>(i - neighborDist, j) != 0)
					{
						v = (2 * neighborDist) / (pBuffer.at<uchar>(i + neighborDist, j) - pBuffer.at<uchar>(i - neighborDist, j));
						v_min_timestamp = min(pBuffer.at<uchar>(i + neighborDist, j), pBuffer.at<uchar>(i - neighborDist, j));
					}
				}

				if ((deltaTimeLeft > 0 && deltaTimeRight > 0) || (deltaTimeLeft < 0 && deltaTimeRight < 0) ||
					(deltaTimeUp > 0 && deltaTimeDown > 0) || (deltaTimeUp < 0 && deltaTimeDown < 0))
				{
					u = 0;
					v = 0;
				}

				if (abs(u_min_timestamp - v_min_timestamp) > timeThreshold)
				{
					u = 0;
					v = 0;
				}
				for (int k = 0; k <= 2 * neighborDist; k++) // Row check for timestamps
				{
					int row = i - neighborDist + k;
					if (row < 0)
						row = 0;
					int currentTimeStamp = pBuffer.at<uchar>(row, j);
					//cout << "currentTimeStamp: " << currentTimeStamp << endl;
					if (timeStampCenter < minTime || timeStampCenter>maxTime)
					{
						u = 0;
						v = 0;
						break;
					}
				}
				for (int k = 0; k <= 2 * neighborDist; k++) // Col check for timestamps
				{
					int col = j - neighborDist + k;
					if (col < 0)
						col = 0;
					int currentTimeStamp = pBuffer.at<uchar>(i, col);
					if (timeStampCenter < minTime || timeStampCenter>maxTime)
					{
						u = 0;
						v = 0;
						break;
					}
				}

				if (abs(u) >= velocityThresMax)
				{
					u = 0;
				}
				if (abs(v) >= velocityThresMax)
				{
					v = 0;
				}

				float velocity = sqrt(u*u + v*v);

				if (velocity <= velocityThresMin)
				{
					u = 0;
					v = 0;
				}
				if (velocity > 255)
					velocity = 255;
				speedBuffer.at<uchar>(i, j) = velocity * 500;

				float theta = 0;
				theta = atan2(v, u) * 180 / CV_PI;

				if (theta < 0)
				{
					theta += 360;
				}
				dirBuffer.at<uchar>(i, j) = theta * 255 / 360;
			}
		}
	}
}
