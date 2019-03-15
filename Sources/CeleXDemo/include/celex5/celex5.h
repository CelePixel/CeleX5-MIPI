/*
* Copyright (c) 2017-2018  CelePixel Technology Co. Ltd.  All rights reserved.
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

using namespace std;

class FrontPanel;
class CeleDriver;
class HHSequenceMgr;
class DataProcessThreadEx;
class DataReaderThread;
class CX5SensorDataServer;
class CommandBase;
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
		Event_Address_Only_Mode = 0,
		Event_Optical_Flow_Mode = 1,
		Event_Intensity_Mode = 2,
		Full_Picture_Mode = 3,
		Full_Optical_Flow_S_Mode = 4,
		Full_Optical_Flow_M_Mode = 6,
	};

	enum emEventPicType {
		EventBinaryPic = 0,
		EventAccumulatedPic = 1,
		EventGrayPic = 2,
		EventCountPic = 3,
		EventDenoisedBinaryPic = 4,
		EventSuperimposedPic = 5,
		EventDenoisedCountPic = 6
	};

	enum emFullPicType {
		Full_Optical_Flow_Pic = 0,
		Full_Optical_Flow_Speed_Pic = 1,
		Full_Optical_Flow_Direction_Pic = 2
	};

	typedef struct CfgInfo
	{
		std::string name;
		uint32_t    min;
		uint32_t    max;
		uint32_t    value;
		uint32_t    step;
		int16_t     high_addr;
		int16_t     middle_addr;
		int16_t     low_addr;
	} CfgInfo;

	typedef struct BinFileAttributes
	{
		uint8_t    data_type; //bit0: 0: fixed mode; 1: loop mode; bit1: 0: no IMU data; 1: has IMU data
		uint8_t    loopA_mode;
		uint8_t    loopB_mode;
		uint8_t    loopC_mode;
		uint8_t    event_data_format;
		uint8_t    hour;
		uint8_t    minute;
		uint8_t    second;
		uint32_t   package_count;
	} BinFileAttributes;

	CeleX5();
	~CeleX5();

	bool openSensor(DeviceType type);
	bool isSensorReady();
	void pipeOutFPGAData();
	void getMIPIData(vector<uint8_t> &buffer);
	void getMIPIData(vector<uint8_t> &buffer, std::time_t& time_stamp_end, vector<IMURawData>& imu_data);

	DeviceType getDeviceType();

	bool setFpnFile(const std::string& fpnFile);
	void generateFPN(std::string fpnFile);

	void setSensorFixedMode(CeleX5Mode mode);
	CeleX5Mode getSensorFixedMode();

	void setSensorLoopMode(CeleX5Mode mode, int loopNum); //LopNum = 1/2/3
	CeleX5Mode getSensorLoopMode(int loopNum); //LopNum = 1/2/3
	void setLoopModeEnabled(bool enable);
	bool isLoopModeEnabled();

	//------- for fixed mode -------
	uint32_t getFullPicFrameTime();
	void setEventFrameTime(uint32_t value); //unit: microsecond
	uint32_t getEventFrameTime();
	void setOpticalFlowFrameTime(uint32_t value); //hardware parameter
	uint32_t getOpticalFlowFrameTime();

	void setEventShowMethod(EventShowType type, int value);

	//------- for loop mode -------
	void setEventDuration(uint32_t value);
	void setPictureNumber(uint32_t num, CeleX5Mode mode);

	//------- sensor control interfaces -------
	void setThreshold(uint32_t value);
	uint32_t getThreshold();
	void setBrightness(uint32_t value);
	uint32_t getBrightness();
	uint32_t getClockRate(); //unit: MHz
	void setClockRate(uint32_t value); //unit: MHz
	void setISOLevel(uint32_t value);
	uint32_t getISOLevel();

	void setEventDataFormat(int format); //0: format 0; 1: format 1; 2: format 2
	int getEventDataFormat();

	void setRotateType(int type);
	int getRotateType();

	void setEventCountStepSize(uint32_t size);

	//------- get image interfaces -------
	void getFullPicBuffer(unsigned char* buffer);
	cv::Mat getFullPicMat();
	void getEventPicBuffer(unsigned char* buffer, emEventPicType type = EventBinaryPic);
	cv::Mat getEventPicMat(emEventPicType type);

	void getOpticalFlowPicBuffer(unsigned char* buffer, emFullPicType type = Full_Optical_Flow_Pic);
	cv::Mat getOpticalFlowPicMat(emFullPicType type);

	//------- get event data vector -------
	bool getEventDataVector(std::vector<EventData> &vector);
	bool getEventDataVector(std::vector<EventData> &vector, uint64_t& frameNo);
	bool getEventDataVectorEx(std::vector<EventData> &vector, std::time_t& time_stamp);

	//------- record raw data interfaces -------
	void startRecording(std::string filePath);
	void stopRecording();

	CX5SensorDataServer* getSensorDataServer();

	//--- Playback Interfaces ---
	bool openBinFile(std::string filePath);
	bool readPlayBackData(long length = 1968644);
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

	void reset();
	void pauseSensor();
	void restartSensor();

	uint32_t getFullFrameFPS();

	void setAntiFlashlightEnabled(bool enabled);
	void setAutoISPEnabled(bool enable);
	bool isAutoISPEnabled();
	void setISPThreshold(uint32_t value, int num);
	void setISPBrightness(uint32_t value, int num);

	//bit7:0~99, bit6:101~199, bit5:200~299, bit4:300~399, bit3:400~499, bit2:500~599, bit1:600~699, bit0:700~799
	//if rowMask = 240 = b'11110000, 0~399 rows will be closed.
	void setRowDisabled(uint8_t rowMask);

	//Whether to display the images when recording
	void setShowImagesEnabled(bool enable); 

	uint32_t getISOLevelCount();

	//------------------ IMU Data ------------------
	int getIMUData(std::vector<IMUData>& data);

	//------------- Version Information ------------
	std::string getSerialNumber();
	std::string getFirmwareVersion();
	std::string getFirmwareDate();

	//------------- cfgs ------------
	map<string, vector<CfgInfo>> getCeleX5Cfg();
	map<string, vector<CfgInfo>> getCeleX5CfgModified();
	void writeRegister(int16_t addressH, int16_t addressM, int16_t addressL, uint32_t value);
	CfgInfo getCfgInfoByName(string csrType, string name, bool bDefault);
	void writeCSRDefaults(string csrType);
	void modifyCSRParameter(string csrType, string cmdName, uint32_t value);

	//--- for test ---
	int  denoisingMaskByEventTime(const cv::Mat& countEventImg, double timelength, cv::Mat& denoiseMaskImg);
	void saveFullPicRawData();

private:
	bool initializeFPGA();
	bool configureSettings(DeviceType type);
	bool resetConfigureSettings(DeviceType type);
	//for write register
	void wireIn(uint32_t address, uint32_t value, uint32_t mask);
	void writeRegister(CfgInfo cfgInfo);
	void resetPin(bool bReset);
	void setALSEnabled(bool enable);
	//
	void enterCFGMode();
	void enterStartMode();
	void disableMIPI();
	void enableMIPI();
	void clearData();

private:
	FrontPanel*                    m_pFrontPanel;
	CeleDriver*                    m_pCeleDriver;

	HHSequenceMgr*                 m_pSequenceMgr;
	DataProcessThreadEx*           m_pDataProcessThread;
	DataRecorder*                  m_pDataRecorder;
	//
	map<string, vector<CfgInfo>>   m_mapCfgDefaults;
	map<string, vector<CfgInfo>>   m_mapCfgModified;
	//
	unsigned char*                 m_pReadBuffer;
	uint8_t*                       m_pDataToRead;

	std::ifstream                  m_ifstreamPlayback;	//playback

	bool                           m_bLoopModeEnabled;

	uint32_t                       m_uiBrightness;
	uint32_t                       m_uiThreshold;
	uint32_t                       m_uiClockRate;
	uint32_t                       m_uiLastClockRate; // for test
	int                            m_iEventDataFormat;
	uint32_t                       m_uiPackageCount;
	uint32_t                       m_uiTotalPackageCount;
	vector<uint64_t>               m_vecPackagePos;
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
	bool                           m_bHasIMUData;
	bool                           m_bClockAutoChanged;
	uint32_t                       m_uiISOLevelCount; //4 or 6

	string						   m_strFirmwareVer;
};

#endif // CELEX5_H
