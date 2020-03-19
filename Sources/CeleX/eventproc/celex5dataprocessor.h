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

#ifndef CELEX5DATAPROCESSOR_H
#define CELEX5DATAPROCESSOR_H

#include "../include/celex5/celex5.h"
#include "../include/celex5/celex5processeddata.h"
#include "../include/celex5/celex5datamanager.h"

#define MAX_BUFFER_NUM    2

class CeleX5DataProcessor
{
public:
	CeleX5DataProcessor();
	~CeleX5DataProcessor();

	void getFullPicBuffer(uint8_t* buffer);
	void getFullPicBuffer(uint8_t* buffer, std::time_t& timestamp);
	//
	void getEventPicBuffer(uint8_t* buffer, CeleX5::EventPicType type);
	void getEventPicBuffer(uint8_t* buffer, std::time_t& timestamp, CeleX5::EventPicType type);
	//
	void getOpticalFlowPicBuffer(uint8_t* buffer, CeleX5::OpticalFlowPicType type);
	void getOpticalFlowPicBuffer(uint8_t* buffer, std::time_t& timestamp, CeleX5::OpticalFlowPicType type);

	bool getEventDataVector(std::vector<EventData> &vecEvent);
	bool getEventDataVector(std::vector<EventData> &vecEvent, uint32_t& frameNo);
	bool getEventDataVector(std::vector<EventData> &vecEvent, uint32_t& frameNo, std::time_t& timestamp);

	void processMIPIData(uint8_t* pData, uint32_t dataSize, std::time_t timestampEnd, std::vector<IMURawData>& imuData);

	void disableFrameModule();
	void enableFrameModule();
	bool isFrameModuleEnabled();

	void disableEventStreamModule();
	void enableEventStreamModule();
	bool isEventStreamEnabled();

	void disableIMUModule();
	void enableIMUModule();
	bool isIMUModuleEnabled();

	void disableEventDenoising();
	void enableEventDenoising();
	bool isEventDenoisingEnabled();

	void disableFrameDenoising();
	void enableFrameDenoising();
	bool isFrameDenoisingEnabled();

	void disableEventOpticalFlow();
	void enableEventOpticalFlow();
	bool isEventOpticalFlowEnabled();

	/*
	* Enable/Disable the Event Count Slice
	*/
	void disableEventCountSlice();
	void enableEventCountSlice();
	bool isEventCountSliceEnabled();

	CX5SensorDataServer *getSensorDataServer();
	CeleX5ProcessedData *getProcessedData();

	bool setFpnFile(const std::string& fpnFile);
	void generateFPN(std::string filePath);

	CeleX5::CeleX5Mode getSensorFixedMode();
	CeleX5::CeleX5Mode getSensorLoopMode(int loopNum);
	void setSensorFixedMode(CeleX5::CeleX5Mode mode);
	void setSensorLoopMode(CeleX5::CeleX5Mode mode, int loopNum);
	void setLoopModeEnabled(bool enable);
	void setISOLevel(uint32_t value);

	void setMIPIDataFormat(int format);

	void setEventFrameTime(uint32_t value, uint32_t clock);
	uint32_t getEventFrameTime();

	void setEventCountSliceNum(uint32_t value);
	uint32_t getEventCountSliceNum();

	void setEventShowMethod(EventShowType type, int value);
	EventShowType getEventShowMethod();
	void setEventFrameStartPos(uint32_t value);
	void setRotateType(int type);
	int  getRotateType();
	void setEventCountStep(uint32_t step);
	uint32_t getEventCountStep();
	int getIMUData(std::vector<IMUData>& data);

	void saveFullPicRawData();
	void resetTimestamp();
	uint32_t getEventRate();
	uint32_t getEventRatePerFrame();

private:
	void processFullPicData(uint8_t* pData, int dataSize, std::time_t timestampEnd);
	void parseEventDataFormat0(uint8_t* pData, int dataSize, std::time_t timestampEnd); //Format0: 24-bit packet with ADC data (CSR_73=2'b00)
	void parseEventDataFormat1(uint8_t* pData, int dataSize, std::time_t timestampEnd); //Format1: 28-bit packet with ADC data (CSR_73=2'b01)
	void parseEventDataFormat2(uint8_t* pData, int dataSize, std::time_t timestampEnd); //Format2: 14-bit packet without ADC data (CSR_73=2'b10)
	//
	void parseIMUData(std::time_t timestamp);

	void checkIfShowImage(); //only for mipi
	bool createImage(std::time_t timestampEnd);
	void generateFPNimpl();
	void loadOpticalFlowFPN();

	void calculateDenoisedBuffer(uint8_t* pDesBuffer1, uint8_t* pDesBuffer2, uint8_t* pSrcBuffer, int neighbours);

	void calDirectionAndSpeed(int i, int j, uint32_t* pBuffer, uint8_t* &speedBuffer, uint8_t* &dirBuffer);
	void calDirectionAndSpeed(uint32_t* pBuffer, uint8_t* &speedBuffer, uint8_t* &dirBuffer);

	void saveFullPicRawData(uint8_t* pData);

	bool findModeInLoopGroup(CeleX5::CeleX5Mode mode);

	void processMIPIEventTimestamp();
	void saveIntensityEvent(int col, int adc12bit, int adc8bit);
	void saveOpticalFlowEvent(int col, int adc12bit, int adc8bit);
	void saveFormat2Event(int col, int adc); //save event into buffer and event vector.
	int  getCurrentIndex(int initIndex);

	void denoisedPerRow(bool bHasADC);
	void calEventCountSlice(uint8_t* pEventCountSliceBuffer);

private:
	CeleX5ProcessedData*     m_pCX5ProcessedData;
	CX5SensorDataServer*     m_pCX5Server;
	//
	uint8_t*                 m_pEventCountBuffer;
	uint32_t*                m_pEventADCBuffer;
	uint32_t*                m_pLastADC;
	//for fpn
	long*                    m_pFpnGenerationBuffer;
	int*                     m_pFpnBuffer;
	int*                     m_pFpnBufferOF;
	//
	uint8_t*                 m_pFullPic[MAX_BUFFER_NUM];
	//
	uint8_t*                 m_pEventBinaryPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pEventDenoisedBinaryPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pEventCountPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pEventDenoisedCountPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pEventGrayPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pEventAccumulatedPic;
	uint8_t*	             m_pEventSuperimposedPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pEventCountSlicePic[MAX_BUFFER_NUM];
	//
	uint8_t*                 m_pOpticalFlowPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pOpticalFlowSpeedPic[MAX_BUFFER_NUM];
	uint8_t*                 m_pOpticalFlowDirectionPic[MAX_BUFFER_NUM];

	uint8_t*                 m_pEventCountSlice[10];
	//
	CeleX5::CeleX5Mode       m_emCurrentSensorMode;
	CeleX5::CeleX5Mode       m_emSensorFixedMode;
	CeleX5::CeleX5Mode       m_emSensorLoopAMode;
	CeleX5::CeleX5Mode       m_emSensorLoopBMode;
	CeleX5::CeleX5Mode       m_emSensorLoopCMode;
	//
	std::string              m_strFpnFilePath;
	//
	uint32_t                 m_uiPixelCount;
	uint32_t                 m_uiPixelCountForEPS; //For event rate
	uint32_t                 m_uiEventNumberEPS; //The number of events that being produced per second
	uint32_t                 m_uiEventTCounter; //This value will be reset after the end of a frame
	uint32_t                 m_uiEventTCounterTotal; //This value won't be reset, it's a monotonically increasing value
	uint32_t                 m_uiEventTCounterEPS; //This value will be reset after the end of a second
	uint32_t                 m_uiEventRowCycleCount;

	uint32_t                 m_uiEventTCountForShow;
	uint32_t                 m_uiEventTCountForRemove;//The data between 0 and m_uiEventTCountForRemove will be removed
	uint32_t                 m_uiEventTCountForEPS; //The number of T units corresponding to 1s.
	uint32_t                 m_uiEventCountForShow;  
	uint32_t                 m_uiEventRowCycleCountForShow;

	uint32_t                 m_uiISOLevel; //range: 1 ~ 6
	EventShowType            m_emEventShowType;

	int32_t                  m_iLastRowTimestamp;
	int32_t                  m_iRowTimestamp;
	int32_t                  m_iLastRow;
	int32_t                  m_iCurrentRow;
	uint32_t                 m_uiRowCount;
	uint32_t                 m_uiEventCountSliceNum;

	int                      m_iFpnCalculationTimes;
	uint32_t                 m_uiEventFrameTime;

	bool                     m_bIsGeneratingFPN;
	bool                     m_bLoopModeEnabled;
	int						 m_iRotateType;	//rotate param

	std::vector<EventData>   m_vecEventData;
	std::vector<EventData>   m_vecEventDataPerRow;

	int                      m_iMIPIDataFormat;
	uint16_t                 m_uiEventTUnitN; //Numerator
	uint16_t                 m_uiEventTUnitDList[16]; //Denominator
	uint16_t                 m_uiCurrentEventTUnitD;
	
	std::ofstream            m_ofLogFile;
	uint32_t                 m_uiEventFrameNo;
	uint32_t                 m_uiEOTrampNo;
	uint32_t                 m_uiPackageTCounter;
	uint32_t                 m_uiEventCountStep;

	std::time_t              m_lCurrentEventFrameTimestamp;
	std::time_t              m_lLastPackageTimestamp;
	std::time_t              m_lFullFrameTimestamp;
	std::time_t              m_lEventFrameTimestamp;
	std::time_t              m_lOpticalFrameTimestamp;

	std::vector<IMUData>     m_vectorIMUData;
	std::vector<IMURawData>  m_vectorIMURawData;

	bool                     m_bSaveFullPicRawData;
	//
	bool                     m_bFrameModuleEnabled;
	bool                     m_bEventStreamEnabled;
	bool                     m_bIMUModuleEnabled;
	bool                     m_bEventDenoisingEnabled;
	bool                     m_bFrameDenoisingEnabled;
	bool                     m_bEventCountDensityEnabled;
	bool                     m_bEventOpticalFlowEnabled;
	//
	int                      m_iLastLoopNum;
	CeleX5::CeleX5Mode       m_emLastLoopMode;
	int                      m_iFPNIndexForAdjustPic;
	uint32_t                 m_uiMinInPixelTimestamp;
	uint32_t                 m_uiMaxInPixelTimestamp;

	bool                     m_bStartGenerateOFFPN;
	uint16_t*                m_pOFFPNEventLatestValue;
};

#endif // CELEX5DATAPROCESSOR_H
