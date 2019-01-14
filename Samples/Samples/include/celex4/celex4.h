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

#ifndef CELEX_H
#define CELEX_H

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

#include <stdint.h>
#include <vector>
#include <map>
#include <list>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "../celextypes.h"
#define IMU_DATA_MAX_SIZE    1000

enum emSensorMode {
	FullPictureMode = 0,
	EventMode = 1,
	FullPic_Event_Mode = 2
};

enum emEventPicMode {
	EventBinaryPic = 0,
	EventAccumulatedPic = 1,
	EventGrayPic = 2,
	EventSuperimposedPic = 3,
	EventDenoisedBinaryPic = 4,
	EventDenoisedGrayPic = 5,
	EventCountPic = 6,
	//EventDenoisedByTimeBinaryPic = 7,
	//EventDenoisedByTimeGrayPic = 8
};

//for bin file reader
typedef struct BinFileAttributes
{
	int hour;
	int minute;
	int second;
	emSensorMode mode;
	long length;
}BinFileAttributes;

typedef struct FrameData
{
	std::vector<EventData> vecEventData;
	uint64_t frameNo;
}FrameData;

typedef struct IMUData {
	//int16_t       x_GYROS;
	//int16_t       y_GYROS;
	//int16_t       z_GYROS;
	//uint32_t      t_GYROS;
	//int16_t       x_ACC;
	//int16_t       y_ACC;
	//int16_t       z_ACC;
	//uint32_t      t_ACC;
	//int16_t       x_GYROS_OFST;
	//int16_t       y_GYROS_OFST;
	//int16_t       z_GYROS_OFST;
	//uint32_t      t_GYROS_OFST;
	//int16_t       x_ACC_OFST;
	//int16_t       y_ACC_OFST;
	//int16_t       z_ACC_OFST;
	//uint32_t      t_ACC_OFST;
	double			x_GYROS;
	double			y_GYROS;
	double			z_GYROS;
	uint32_t		t_GYROS;
	double			x_ACC;
	double			y_ACC;
	double			z_ACC;
	uint32_t		t_ACC;
	double			x_MAG;
	double			y_MAG;
	double			z_MAG;
	uint32_t		t_MAG;
	double			x_TEMP;
	uint64_t		frameNo;
} IMUData;

class FrontPanel;
class HHSequenceMgr;
class HHSequenceSlider;
class DataProcessThread;
class DataReaderThread;
class CeleX4ProcessedData;
class CX4SensorDataServer;
class EventProcessing;
class FPGADataProcessor;
class DataRecorder;
class CELEX_EXPORTS CeleX4
{
public:
	enum emDeviceType {
		Sensor = 0,
		FPGA,
		SensorAndFPGA
	};
	enum emAdvancedBiasType {
		EVT_VL = 0,
		EVT_VH,
		ZONE_MINUS,
		ZONE_PLUS,
		REF_MINUS,
		REF_PLUS,
		REF_MINUS_H,
		REF_PLUS_H,
		EVT_DC,
		LEAK,
		CDS_DC,
		CDS_V1,
		PixBias,
		Gain,
		Clock,
		Resolution
	};
	enum ErrorCode {
		NoError = 0,
		InitializeFPGAFailed,
		PowerUpFailed,
		ConfigureFailed
	};
	typedef struct TimeInfo
	{
		int hour;
		int minute;
		int second;
	} TimeInfo;

	typedef struct ControlSliderInfo
	{
		std::string name;
		uint32_t    min;
		uint32_t    max;
		uint32_t    value;
		uint32_t    step;
		bool        bAdvanced;
	} ControlSliderInfo;

	CeleX4();
	~CeleX4();

	ErrorCode openSensor(std::string str);
	bool isSensorReady();
	bool isSdramFull();

	//--- Sensor Operating Mode Interfaces ---
	void setSensorMode(emSensorMode mode);
	emSensorMode getSensorMode();

	//--- FPN Interfaces ---
	bool setFpnFile(const std::string& fpnFile);
	void generateFPN(std::string fpnFile);

	void pipeOutFPGAData();
	long getFPGADataSize();
	long readDataFromFPGA(long length, unsigned char *data);

	unsigned char* getFullPicBuffer();
	cv::Mat getFullPicMat();
	unsigned char* getEventPicBuffer(emEventPicMode mode = EventBinaryPic);
	cv::Mat getEventPicMat(emEventPicMode mode);

	bool getEventDataVector(std::vector<EventData> &vector);
	bool getEventDataVector(std::vector<EventData> &vector, uint64_t& frameNo);

	//---------------------------------------------------------------------------------------------test
	void setVecSizeAndOverlap(unsigned long vecSize, unsigned long overlap);
	bool getFixedNumEventDataVec(/*long length, long overlapLen, */std::vector<EventData> &vector, uint64_t& frameNo);

	void setThreshold(uint32_t value);
	uint32_t getThreshold();

	void setContrast(uint32_t value);
	uint32_t getContrast();

	void setBrightness(uint32_t value);
	uint32_t getBrightness();

	//--- Set the lower and upper limits of A Interfaces ---
	void setLowerADC(uint32_t value);
	uint32_t getLowerADC();
	void setUpperADC(uint32_t value);
	uint32_t getUpperADC();

	//--- Sensor Control Interfaces ---
	void resetFPGA();
	void resetSensorAndFPGA();
	void enableADC(bool enable);

	void trigFullPic();

	//--- for clock ---
	uint32_t getClockRate(); //unit: MHz
	void setClockRate(uint32_t value); //unit: MHz

	//--- Set the methods of creating pic frame Interfaces ---
	void setFullPicFrameTime(uint32_t msec); //unit: millisecond
	uint32_t getFullPicFrameTime();
	void setEventFrameTime(uint32_t msec); //unit: millisecond
	uint32_t getEventFrameTime();
	void setFEFrameTime(uint32_t msec); //unit: millisecond
	uint32_t getFEFrameTime();

	//set start ratio and end ratio of the created frame to the event length when sensor works in FullPic_Event mode
	void setFrameLengthRange(float startRatio, float endRatio);

	void setOverlapTime(uint32_t msec); //unit: millisecond
	uint32_t getOverlapTime();
	void setEventFrameParameters(uint32_t frameTime, uint32_t intervalTime);
	void setTimeScale(float scale);
	void setEventCountStepSize(uint32_t size);

	//--- for optical flow ---
	void enableOpticalFlow(bool enable);
	bool isOpticalFlowEnabled();
	void setOpticalFlowLatencyTime(uint32_t msec);
	void setOpticalFlowSliceCount(uint32_t count);
	unsigned char* getOpticalFlowPicBuffer();
	cv::Mat getOpticalFlowPicMat();
	unsigned char* getOpticalFlowDirectionPicBuffer();
	cv::Mat getOpticalFlowDirectionPicMat();
	unsigned char* getOpticalFlowSpeedPicBuffer();
	cv::Mat getOpticalFlowSpeedPicMat();

	//--- Record Sensor Data Interfaces ---
	void startRecording(std::string filePath);
	void stopRecording();
	void startRecordingVideo(std::string filePath, std::string fullPicName, std::string eventName, int fourcc, double fps);
	void stopRecordingVideo();
	//--- Playback Interfaces ---
	bool readPlayBackData(long length = 1968644);
	bool openPlaybackFile(std::string filePath);
	void play();
	void pause();
	long getPlaybackFileSize();
	bool setPlayBackOffset(long offset);
	void saveSelectedBinFile(std::string filePath, long fromPos, long toPos, int hour, int minute, int second);
	PlaybackState getPlaybackState();
	void setPlaybackState(PlaybackState state);

	BinFileAttributes getAttributes(std::string binFile);
	void convertBinToAVI(std::string binFile, emEventPicMode picMode, uint32_t frameTime, uint32_t intervalTime, cv::VideoWriter writer);
	void convertBinToAVI(std::string binFile, cv::VideoWriter writer);

	void enableAutoAdjustBrightness(bool enable);
	//--- IMU Data ---
	int getIMUDataSize();
	int getIMUData(int count, std::vector<IMUData>& data);
	void setIMUIntervalTime(uint32_t value);
	int getIMUData(std::vector<IMUData>& data);

	bool denoisingByTimeInterval(std::vector<EventData> vec, cv::Mat &mat);
	bool denoisingAndCompresing(std::vector<EventData> vec, float compressRatio, cv::Mat &mat);

	void setResetLength(uint32_t value);

	void clearData();
	TimeInfo getRecordedTime();

	CeleX4ProcessedData* getSensorDataObject();
	CX4SensorDataServer* getSensorDataServer();
	std::vector<ControlSliderInfo> getSensorControlList();

	void pauseThread(bool pause);

	uint32_t getPageCount();
	uint32_t getMeanIntensity();

	unsigned long getSpecialEventCount();
	void setSpecialEventCount(unsigned long count);

	std::vector<int> getDataLengthPerSpecial();
	std::vector<unsigned long> getEventCountListPerSpecial();

private:
	bool powerUp();
	bool configureSettings();
	void parseSliderSequence();
	bool excuteCommand(std::string strCommand);
	bool setAdvancedBias(std::string strBiasName);
	bool setAdvancedBias(std::string strBiasName, int value);
	void autoAdjustBrightness();
	//
	bool openBinFile(std::string filePath);

private:
	std::map<std::string, uint32_t>  m_mapSliderNameValue; //All Setting Names & Initial Values
	std::vector<std::string>         m_vecAdvancedNames;   //Advanced Setting Names
	std::vector<ControlSliderInfo>   m_vecSensorControlList;
	std::ifstream                    m_ifstreamPlayback;

	FrontPanel*                      m_pFrontPanel;
	HHSequenceMgr*                   m_pSequenceMgr;
	DataProcessThread*               m_pDataProcessThread;
	DataReaderThread*                m_pDataReaderThread;
	DataRecorder*                    m_pDataRecorder;
	unsigned char*                   m_pReadBuffer;

	uint32_t                         m_uiFullPicFrameTime;
	uint32_t                         m_uiEventFrameTime;
	uint32_t                         m_uiFEFrameTime;
	uint32_t                         m_uiOverlapTime;
	uint32_t                         m_uiClockRate;

	TimeInfo                         m_stTimeRecorded;
	long                             m_lPlaybackFileSize;
	//for test
	uint32_t                         m_uiPageCount;
	bool							 m_bOpticalFlowEnable;
	bool                             m_bReadDataByAPICore;
	bool                             m_bAutoAdjustBrightness;
	//
	std::ifstream					 m_ifstreamBin;
    bool                             m_bRecordingVideo;

	//for denoising and compressing
	long							m_lPreT;
	float							m_fTSum;
	long							m_lCount;
	float							m_fDenoiseCount;
	float							m_fCompressCount;
	int								m_iDelta;
	float							m_fTao;
	cv::Mat							m_MatCompressTemplateImg;
};

#endif // CELEX_H
