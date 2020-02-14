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

#ifndef CELEX5_H
#define CELEX5_H

#include <stdint.h>
#include <vector>
#include <map>
#include <list>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "../celextypes.h"

#ifdef _WIN32
#ifdef CELEX_API_EXPORTS
#define CELEX_EXPORTS __declspec(dllexport)
#else
#define CELEX_EXPORTS __declspec(dllimport)
#endif
#else
#if defined(CELEX_LIBRARY)
#define CELEX_EXPORTS
#else
#define CELEX_EXPORTS
#endif
#endif

class CeleDriver;
class CeleX5DataProcessor;
class CeleX5CfgMgr;
class DataProcessThread;
class CX5SensorDataServer;
class DataRecorder;
class CELEX_EXPORTS CeleX5
{
public:
	enum DeviceType {
		Unknown_Devive = 0,
		CeleX5_MIPI = 1,
		CeleX5_OpalKelly = 2,
	};

	enum CeleX5Mode {
		Unknown_Mode = -1,
		Event_Off_Pixel_Timestamp_Mode = 0,	//Using Event_Off_Pixel_Timestamp_Mode, Event_Address_Only_Mode is deprecated.
		Event_In_Pixel_Timestamp_Mode = 1,	//Using Event_In_Pixel_Timestamp_Mode, Event_Optical_Flow_Mode is deprecated.
		Event_Intensity_Mode = 2,	
		Full_Picture_Mode = 3,
		Optical_Flow_Mode = 4,	//Using Optical_Flow_Mode, Full_Optical_Flow_S_Mode is deprecated.
		Optical_Flow_FPN_Mode = 5,	//Using Optical_Flow_FPN_Mode, Full_Optical_Flow_Test_Mode is deprecated.
		Multi_Read_Optical_Flow_Mode = 6,	//Using Multi_Read_Optical_Flow_Mode, Full_Optical_Flow_M_Mode  is deprecated.
	};

	enum EventPicType {
		EventBinaryPic = 0,
		EventAccumulatedPic = 1,
		EventGrayPic = 2,
		EventCountPic = 3,
		EventDenoisedBinaryPic = 4,
		EventSuperimposedPic = 5,
		EventDenoisedCountPic = 6,
		EventCountDensityPic = 7,
		EventInPixelTimestampPic = 8
	};

	enum OpticalFlowPicType {
		OpticalFlowPic = 0,
		OpticalFlowSpeedPic = 1,
		OpticalFlowDirectionPic = 2
	};

	typedef struct CfgInfo
	{
		uint32_t    min;
		uint32_t    max;
		uint32_t    value;
		uint32_t    step;
		int16_t     highAddr;
		int16_t     middleAddr;
		int16_t     lowAddr;
		std::string name;
	} CfgInfo;

	typedef struct BinFileAttributes
	{
		uint8_t    dataType; //bit0: 0: fixed mode; 1: loop mode; bit1: 0: no IMU data; 1: has IMU data
		uint8_t    loopAMode;
		uint8_t    loopBMode;
		uint8_t    loopCMode;
		uint8_t    eventDataFormat;
		uint8_t    hour;
		uint8_t    minute;
		uint8_t    second;
		uint32_t   packageCount;
	} BinFileAttributes;

	CeleX5();
	~CeleX5();

	bool openSensor(DeviceType type);
	bool isSensorReady();

	/*
	* Get Sensor raw data interfaces
	* If you don't care about IMU data, you can use the first getMIPIData interface, 
	* otherwise you need to use the second getMIPIData interface.
	*/
	void getCeleXRawData(uint8_t* pData, uint32_t& length);
	void getCeleXRawData(uint8_t* pData, uint32_t& length, std::time_t& timestampEnd, std::vector<IMURawData>& imuData);

	/*
	* Parse Sensor raw data interfaces
	* If you don't care about IMU data, you can use the first parseMIPIData interface,
	* otherwise you need to use the second parseMIPIData interface.
	*/
	void parseCeleXRawData(uint8_t* pData, uint32_t dataSize);
	void parseCeleXRawData(uint8_t* pData, uint32_t dataSize, std::time_t timestampEnd, std::vector<IMURawData> imuData);

	/* 
	* Enable/Disable the Create Image Frame module
	* If you just want to obtain (x,y,A,t) array (don't need frame data), you cound disable this function to imporve performance.
	*/
	void disableFrameModule(); 
	void enableFrameModule();
	bool isFrameModuleEnabled();

	/*
	* Enable/Disable the Event Stream module
	* If you just want to event frame (don't need (x,y,A,t) stream), you cound disable this function to imporve performance.
	*/
	void disableEventStreamModule();
	void enableEventStreamModule();
	bool isEventStreamEnabled();

	/*
	* Disable/Enable the IMU module
	* If you don't want to obtain IMU data, you cound disable this function to imporve performance.
	*/
	void disableIMUModule();
	void enableIMUModule();
	bool isIMUModuleEnabled();

	/*
	* Enable/Disable the Event Denoising
	*/
	void disableEventDenoising();
	void enableEventDenoising();
	bool isEventDenoisingEnabled();

	/*
	* Enable/Disable the Frame Denoising
	*/
	void disableFrameDenoising();
	void enableFrameDenoising();
	bool isFrameDenoisingEnabled();

	/*
	* Enable/Disable the Event Count Slice
	*/
	void disableEventCountSlice();
	void enableEventCountSlice();
	bool isEventCountSliceEnabled();

	/*
	* Enable/Disable the Event Optical Flow
	*/
	void disableEventOpticalFlow();
	void enableEventOpticalFlow();
	bool isEventOpticalFlowEnabled();

	/*
	* Get Full-frame pic buffer or mat
	*/
	void getFullPicBuffer(uint8_t* buffer);
	void getFullPicBuffer(uint8_t* buffer, std::time_t& timestamp);
	cv::Mat getFullPicMat();

	/*
	* Get event pic buffer or mat
	*/
	void getEventPicBuffer(uint8_t* buffer, EventPicType type = EventBinaryPic);
	void getEventPicBuffer(uint8_t* buffer, std::time_t& timestamp, EventPicType type = EventBinaryPic);
	cv::Mat getEventPicMat(EventPicType type);

	/*
	* Get optical-flow pic buffer or mat
	*/
	void getOpticalFlowPicBuffer(uint8_t* buffer, OpticalFlowPicType type = OpticalFlowPic);
	void getOpticalFlowPicBuffer(uint8_t* buffer, std::time_t& timestamp, OpticalFlowPicType type = OpticalFlowPic);
	cv::Mat getOpticalFlowPicMat(OpticalFlowPicType type);

	/*
	* Get event data vector interfaces
	*/
	bool getEventDataVector(std::vector<EventData>& vecEvent);
	bool getEventDataVector(std::vector<EventData>& vecEvent, uint32_t& frameNo);
	bool getEventDataVector(std::vector<EventData>& vecEvent, uint32_t& frameNo, std::time_t& timestamp);

	/*
	* Get IMU Data
	*/
	int getIMUData(std::vector<IMUData>& data);

	/*
	* Set and get sensor mode (fixed mode)
	*/
	void setSensorFixedMode(CeleX5Mode mode);
	CeleX5Mode getSensorFixedMode();

	/*
	* Set and get sensor mode (Loop mode)
	*/
	void setSensorLoopMode(CeleX5Mode mode, int loopNum); //LopNum = 1/2/3
	CeleX5Mode getSensorLoopMode(int loopNum); //LopNum = 1/2/3
	void setLoopModeEnabled(bool enable);
	bool isLoopModeEnabled();

	/*
	* Set fpn file to be used in Full_Picture_Mode or Event_Intensity_Mode.
	*/
	bool setFpnFile(const std::string& fpnFile);

	/*
	* Generate fpn file
	*/
	void generateFPN(const std::string& fpnFile);
    void stopGenerateFPN();

	/*
	* Clock
	* By default, the CeleX-5 sensor works at 100 MHz and the range of clock rate is from 20 to 100, step is 10.
	*/
	void setClockRate(uint32_t value); //unit: MHz
	uint32_t getClockRate(); //unit: MHz

	/*
	* Threshold
	* The threshold value only works when the CeleX-5 sensor is in the Event Mode.
	* The large the threshold value is, the less pixels that the event will be triggered (or less active pixels).
	* The value could be adjusted from 50 to 511, and the default value is 171.
	*/
	void setThreshold(uint32_t value);
	uint32_t getThreshold();

	/*
	* Brightness
	* Configure register parameter, which controls the brightness of the image CeleX-5 sensor generated.
	* The value could be adjusted from 0 to 1023.
	*/
	void setBrightness(uint32_t value);
	uint32_t getBrightness();

	/*
	* ISO Level
	*/
	void setISOLevel(uint32_t value);
	uint32_t getISOLevel();
	uint32_t getISOLevelCount();

	/*
	* Get the frame time of full-frame picture mode
	*/
	uint32_t getFullPicFrameTime();

	/*
	* Set and get event frame time
	*/
	void setEventFrameTime(uint32_t value); //unit: microsecond
	uint32_t getEventFrameTime();

	/*
	* Set and get frame time of optical-flow mode
	*/
	void setOpticalFlowFrameTime(uint32_t value); //hardware parameter, unit: microsecond
	uint32_t getOpticalFlowFrameTime();

	/*
	* Set Event Count Slice Number
	*/
	void setEventCountSliceNum(uint32_t value);
	uint32_t getEventCountSliceNum();

	/* 
	* Loop mode: mode duration
	*/
	void setEventDuration(uint32_t value);
	void setPictureNumber(uint32_t num, CeleX5Mode mode);

	/*
	* Control Sensor interfaces
	*/
	void reset(); //soft reset sensor
	void pauseSensor();
	void restartSensor();
	void stopSensor();

	/*
	* Get the serial number of the sensor, and each sensor has a unique serial number.
	*/
	std::string getSerialNumber();

	/*
	* Get the firmware version of the sensor.
	*/
	std::string getFirmwareVersion();

	/*
	* Get the release date of firmware.
	*/
	std::string getFirmwareDate();
	
	/*
	* Set and get event show method
	*/
	void setEventShowMethod(EventShowType type, int value);
	EventShowType getEventShowMethod();

	/*
	* Set and get rotate type
	*/
	void setRotateType(int type);
	int getRotateType();

	/*
	* Set and get event count stop
	*/
	void setEventCountStepSize(uint32_t size);
	uint32_t getEventCountStepSize();

	/*
	* bit7:0~99, bit6:101~199, bit5:200~299, bit4:300~399, bit3:400~499, bit2:500~599, bit1:600~699, bit0:700~799
	* if rowMask = 240 = b'11110000, 0~399 rows will be closed.
	*/
	void setRowDisabled(uint8_t rowMask);

	/*
	* Whether to display the images when recording
	*/
	void setShowImagesEnabled(bool enable);
	
	/* 
	* Set and get event data format
	*/
	void setEventDataFormat(int format); //0: format 0; 1: format 1; 2: format 2
	int getEventDataFormat();

	void setEventFrameStartPos(uint32_t value); //unit: minisecond

	/*
	* Disable/Enable AntiFlashlight function.
	*/
	void setAntiFlashlightEnabled(bool enabled);

	/*
	* Disable/Enable Auto ISP function.
	*/
	void setAutoISPEnabled(bool enable);
	bool isAutoISPEnabled();
	void setISPThreshold(uint32_t value, int num);
	void setISPBrightness(uint32_t value, int num);

	/*
	* Start/Stop recording raw data.
	*/
	void startRecording(const std::string& filePath);
	void stopRecording();

	/*
	* Playback Interfaces
	*/
	bool openBinFile(const std::string& filePath);
	bool readBinFileData();
	uint32_t getTotalPackageCount();
	uint32_t getCurrentPackageNo();
	void setCurrentPackageNo(uint32_t value);
	BinFileAttributes getBinFileAttributes();
	void replay();
	void play();
	void pause();
	PlaybackState getPlaybackState();
	void setPlaybackState(PlaybackState state);
	void setIsPlayBack(bool state);
	void saveBinFile(std::string strPath, int startPackageCount, int endPackageCount);

	CX5SensorDataServer* getSensorDataServer();
	
	DeviceType getDeviceType();

	uint32_t getFullFrameFPS();

	/*
	* Obtain the number of events that being produced per second.
	* Unit: events per second
	*/
	uint32_t getEventRate(); 
	uint32_t getEventRatePerFrame();

	int getALSValue();

	/*
	* Sensor Configures
	*/
	std::map<std::string, std::vector<CfgInfo> > getCeleX5Cfg();
	std::map<std::string, std::vector<CfgInfo> > getCeleX5CfgModified();
	void writeRegister(int16_t addressH, int16_t addressM, int16_t addressL, uint32_t value);
	CfgInfo getCfgInfoByName(const std::string& csrType, const std::string& name, bool bDefault);
	void writeCSRDefaults(const std::string& csrType);
	void modifyCSRParameter(const std::string& csrType, const std::string& cmdName, uint32_t value);

	//--- for test ---
	int  denoisingMaskByEventTime(const cv::Mat& countEventImg, double timelength, cv::Mat& denoiseMaskImg);
	void saveFullPicRawData();

	void calDirectionAndSpeedEx(cv::Mat pBuffer, cv::Mat &speedBuffer, cv::Mat &dirBuffer);

private:
	bool configureSettings(DeviceType type);
	//for write register
	void wireIn(uint32_t address, uint32_t value, uint32_t mask);
	void writeRegister(CfgInfo cfgInfo);
	void setALSEnabled(bool enable);
	bool isALSEnabled();
	//
	void enterCFGMode();
	void enterStartMode();
	void disableMIPI();
	void enableMIPI();
	void clearData();

private:
	CeleDriver*                    m_pCeleDriver;
	CeleX5DataProcessor*           m_pDataProcessor;
	CeleX5CfgMgr*                  m_pCeleX5CfgMgr;
	DataProcessThread*             m_pDataProcessThread;
	DataRecorder*                  m_pDataRecorder;
	//
	std::map<std::string, std::vector<CfgInfo> >  m_mapCfgDefaults;
	std::map<std::string, std::vector<CfgInfo> >  m_mapCfgModified;
	//
	uint8_t*                       m_pReadBuffer;
	uint8_t*                       m_pDataToRead;

	std::ifstream                  m_ifstreamPlayback;	//playback

	bool                           m_bLoopModeEnabled;
	bool                           m_bALSEnabled;

	uint32_t                       m_uiBrightness;
	uint32_t                       m_uiThreshold;
	uint32_t                       m_uiClockRate;
	uint32_t                       m_uiLastClockRate;
	int                            m_iEventDataFormat;
	uint32_t                       m_uiPackageCount;
	uint32_t                       m_uiTotalPackageCount;
	std::vector<uint64_t>          m_vecPackagePos;
	bool                           m_bFirstReadFinished;
	BinFileAttributes              m_stBinFileHeader;
	DeviceType                     m_emDeviceType;
	uint32_t                       m_uiPackageCounter;
	uint32_t                       m_uiPackageCountPS;
	uint32_t                       m_uiPackageTDiff;
	uint32_t                       m_uiPackageBeginT;
	bool                           m_bAutoISPEnabled;
	uint32_t                       m_arrayISPThreshold[3];
	uint32_t                       m_arrayBrightness[4];
	uint32_t                       m_uiAutoISPRefreshTime;
	uint32_t                       m_uiISOLevel; //range: 1 ~ 6

	uint32_t                       m_uiOpticalFlowFrameTime;

	int							   m_iRotateType;	//rotate param
	bool                           m_bClockAutoChanged;
	uint32_t                       m_uiISOLevelCount; //4 or 6

	bool                           m_bSensorReady;
	bool                           m_bShowImagesEnabled;
	bool                           m_bAutoISPFrofileLoaded;
};

#endif // CELEX5_H
