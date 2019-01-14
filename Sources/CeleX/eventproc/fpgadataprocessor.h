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

#ifndef FPGADATAPROCESSOR_H
#define FPGADATAPROCESSOR_H

#include <queue>
#include <string>
#include <list>
#include <stdint.h>
#include <stdio.h>
#include "../base/dataqueue.h"
#include "../include/celex4/celex4.h"

#include <opencv2/core/core.hpp>        
#include <opencv2/highgui/highgui.hpp>        
#include <opencv2/imgproc/imgproc.hpp> 

extern bool				isCreateImage;
extern cv::VideoWriter  g_cvVideoWriterFullPic;
extern cv::VideoWriter  g_cvVideoWriterEvent;
extern bool				isReadBin;
extern FrameData        g_frameData;
extern FrameData        g_fixedVecData;
//extern uint64_t         g_ulEventFrameNo;
//extern uint64_t         g_ulFixedEventNo;
extern std::vector<EventData>  g_eventData;

typedef struct PixelData
{
	unsigned int    index;
	unsigned int    value;
    unsigned int    t;
} PixelData;

typedef struct EventRowData
{
	unsigned int t;
	unsigned int specialNo;
	std::queue<int> indexs;
	std::queue<int> adcs;
} EventRowData;

class CeleX4ProcessedData;
class CX4SensorDataServer;
class FPGADataProcessor
{
public:
    FPGADataProcessor();
    ~FPGADataProcessor();
    bool setFpnFile(const std::string& fpnFile);
    void processData(unsigned char* data, long length);
    bool processEvent(unsigned char* data); // return true for normal event, false for special event
	bool processRowEvent(unsigned char* data);
    void processColEvent(unsigned char* data);
    void createImage();
    void cleanEventBuffer();
    void setSensorMode(emSensorMode mode);
    emSensorMode getSensorMode();
    void generateFPN(std::string filePath);
    void adjustBrightness();
    void setUpperADC(uint32_t value);
	uint32_t getUpperADC();
    void setLowerADC(uint32_t value);
	uint32_t getLowerADC();
	//
	void setTimeSlice(uint32_t msec); //unit: millisecond
	void setOverlapTime(uint32_t msec); //unit: millisecond
	//set start ratio and end ratio of the created frame to the event length when sensor works in FullPic_Event mode
	void setFrameLengthRange(float startRatio, float endRatio);
	void setFEFrameTime(uint32_t msec);
	//
	CeleX4ProcessedData *getSensorDataObject();
	CX4SensorDataServer *getSensorDataServer();
	//--- for clock ---
	void setClockRate(uint32_t value); //unit: MHz
	void setFPGATimeCycle(uint32_t value);
	//--- for multi-slice ---
	void enableMultiSlice(bool enable);
	bool isMultiSliceEnabled();
	void setMultiSliceTime(uint32_t msec);
	void setMultiSliceCount(uint32_t count);

	unsigned long getSpecialEventCount();
	void setSpecialEventCount(unsigned long count);
	std::vector<int> getDataLengthPerSpecial();
	std::vector<unsigned long> getEventCountListPerSpecial();
	uint32_t getMeanIntensity();

	BinFileAttributes getBinFileAttributes(std::string binFile);	
	void setTimeScale(float scale);
	void setVecSizeAndOverlap(unsigned long vecSize, unsigned long overlap); //------------#####################################---------------------------

	void setEventCountStepSize(uint32_t stepSize);
	unsigned int normalizeADC(unsigned int adc);
	//
	int calculateDenoiseScore(unsigned char* pBuffer, unsigned int pos);
	int calculateDenoiseScore(unsigned char* pBuffer, unsigned char* pBufferPre, unsigned int pos);

	void denoisingByTimeInterval(std::vector<EventData> vec, unsigned char* binaryBuffer, emEventPicMode picMode);
	int getIMUDataSize();
	int getIMUData(int count, std::vector<IMUData>& data);
	int getIMUData(std::vector<IMUData>& data);
    void setPlaybackState(PlaybackState state);

private:
    void generateFPNimpl();
    void adjustBrightnessImpl();
	void calDirectionAndSpeed(); //for multi-slice

private:
    unsigned char*           m_pFullPicBuffer;
    unsigned char*           m_pEventPicBuffer;
	unsigned char*           m_pBufferMotion;  //for for ABC mode
	unsigned char*           m_pBufferOpticalFlow; //for ABC mode
	unsigned char*           m_pLastEventPicBuffer; //for denoise

	uint16_t*                m_pLastADC;
	//for fpn
    long*                    m_pFpnGenerationBuffer;
    int*                     m_pFpnBuffer;

    long*                    m_pOverlapEventBuffer;
	//
	CeleX4ProcessedData*     m_pCX4ProcessedData;
	CX4SensorDataServer*     m_pCX4Server;

	std::string              m_strFpnFilePath;

	emSensorMode             m_emSensorMode;
    //
    int                      m_iFpnCalculationTimes;
	unsigned int             m_uiFullPicPixelCount; //for ABC mode
	unsigned int             m_uiOpticalFlowPixelCount; //for ABC mode
	unsigned int             m_uiEventPixelCount; //for ABC mode
	//for event
	unsigned long            m_ulEventTCounter;
	//
	unsigned long            m_ulEventCountPerSecond;
	unsigned long            m_ulEventCountPerSpecial;
	unsigned long            m_ulEventCountTotal;
	std::vector<unsigned long>    m_vecEventCountPerSpecial;
    //
    uint32_t                 m_uiUpperADC;
    uint32_t                 m_uiLowerADC;
    uint32_t                 m_uiTimeSlice;
	uint32_t                 m_uiTimeSliceValue; //ms
	uint32_t                 m_uiOverlapTime;
	uint32_t                 m_uiClockRate;
	uint32_t                 m_uiFPGATimeCycle; //Version 2.x: 2^17=131072; Version 1.x: 2^18=262144
	//
	uint32_t                 m_uiFrameStartT;
	uint32_t                 m_uiFrameEndT;
	uint32_t                 m_uiFEEventTCounter;
	uint32_t                 m_uiFEFrameTime;

	bool                     m_bIsGeneratingFPN;
	bool                     m_bAdjustBrightness;

	//for overlap
	unsigned long            m_ulOverlapEventTCounter;
	//
	uint32_t                 m_uiMeanIntensity;

	//for multi-slice
	bool                     m_bMultiSliceEnabled;
	bool                     m_bNeedResetMultiSliceData;
	uint32_t                 m_uiMultiSliceTime;
	uint32_t                 m_uiMultiSliceCount;
	unsigned long            m_ulMultiSliceTCounter;
	long*                    m_pMultiSliceBuffer;
	unsigned char*           m_pMultiSliceGrayBuffer;

	//for playback
	std::vector<int>         m_vecDataLength;
	unsigned long            m_ulSpecialEventCount;
	unsigned long            m_ulSpecialByteSize;
	PlaybackState            m_emPlaybackState;

	//
	std::ifstream			 m_ifstreamBin;
	uint32_t				 m_uiTimeScale;
	uint32_t				 m_uiTimeStamp;
	unsigned long			 m_ulTimeStampCount;

	unsigned long			 m_ulVecSize;
	unsigned long			 m_ulOverlapLen;

	uint32_t				 m_uiEventCountStepSize;
	std::vector<EventData>	 m_vectorEventData;
	std::vector<EventData>   m_fixedNumVecEventData;
	
	std::vector<IMUData>     m_vectorIMUData;
	std::vector<IMUData>     m_vectorIMUData_cb;

	long preT = 0.0;
	float tSum = 0.0;
	long count = 0.0;
};

#endif // FPGADATAPROCESSOR_H
