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

#ifndef CELEX5DATAPROCESSOR_H
#define CELEX5DATAPROCESSOR_H

#include "../include/celex5/celex5.h"
#include "../include/celex5/celex5processeddata.h"
#include "../include/celex5/celex5datamanager.h"
#include "../eventproc/celex5datareader.h"

#define FORMAT1_T_MAX    65536 // 2^16
#define FORMAT2_T_MAX    4096  // 2^12

class CeleX5DataProcessor
{
public:
	typedef struct CeleX5PixelData
	{
		uint32_t    index;
		uint16_t    adc;
		uint16_t    count;
		uint32_t    t;
	} CeleX5PixelData;

	CeleX5DataProcessor();
	~CeleX5DataProcessor();

	void getFullPicBuffer(unsigned char* buffer);
	void getEventPicBuffer(unsigned char* buffer, CeleX5::emEventPicType type);
	void getOpticalFlowPicBuffer(unsigned char* buffer, CeleX5::emFullPicType type);

	bool getEventDataVector(std::vector<EventData> &vector);
	bool getEventDataVector(std::vector<EventData> &vector, uint64_t& frameNo);

	void processData(unsigned char* data, long length);
	bool processEvent(unsigned char* data); // return true for normal event, false for special event

	void processMIPIData(uint8_t* pData, int dataSize);

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
	void setEventShowMethod(EventShowType type, int value);
	void setRotateType(int type);

private:
	bool processRowEvent(unsigned char* data);
	bool processColEvent(unsigned char* data);
	//
	void processFullPicData(uint8_t* pData, int dataSize);
	void parseEventDataFormat0(uint8_t* pData, int dataSize);
	void parseEventDataFormat1(uint8_t* pData, int dataSize);
	void parseEventDataFormat2(uint8_t* pData, int dataSize);
	//
	bool createImage();
	unsigned int normalizeADC(unsigned int adc);
	void generateFPNimpl();
	int calculateDenoiseScore(unsigned char* pBuffer, unsigned int pos);
	int calMean(unsigned char* pBuffer, unsigned int pos);
	void calDirectionAndSpeed(int i, int j, unsigned char* pBuffer, unsigned char* &speedBuffer, unsigned char* &dirBuffer);
	int getCurrentIndex(int initIndex);

	inline void processMIPIEventTimeStamp() 
	{
		int diffT = m_iRowTimeStamp - m_iLastRowTimeStamp;
		if (diffT < 0)
		{
			if (1 == m_iMIPIDataFormat)
				diffT += FORMAT1_T_MAX;
			else if (2 == m_iMIPIDataFormat)
				diffT += FORMAT2_T_MAX;
		}	
		/*if (diffT > 1)
			cout << __FUNCTION__ << ": T is not continuous!" << endl;*/
		if (m_iLastRowTimeStamp != -1 && diffT < 2)
			m_uiEventTCounter += diffT;
	}

private:
	CeleX5DataReader*        m_pFPGADataReader;
	CeleX5ProcessedData*     m_pCX5ProcessedData;
	CX5SensorDataServer*     m_pCX5Server;
	//
	unsigned char*           m_pEventCountBuffer;
	unsigned char*           m_pEventADCBuffer;
	uint16_t*                m_pLastADC;
	//for fpn
	long*                    m_pFpnGenerationBuffer;
	int*                     m_pFpnBuffer;
	//
	CeleX5::CeleX5Mode       m_emCurrentSensorMode;
	CeleX5::CeleX5Mode       m_emSensorFixedMode;
	CeleX5::CeleX5Mode       m_emSensorLoopAMode;
	CeleX5::CeleX5Mode       m_emSensorLoopBMode;
	CeleX5::CeleX5Mode       m_emSensorLoopCMode;
	//
	string                   m_strFpnFilePath;
	//
	uint32_t                 m_uiPixelCount;
	uint32_t                 m_uiEventTCounter;
	uint32_t                 m_uiEventRowCycleCount;

	uint32_t                 m_uiEventTCountForShow;
	uint32_t                 m_uiEventCountForShow;
	uint32_t                 m_uiEventRowCycleCountForShow;

	uint32_t                 m_uiISOLevel; //range: 1 ~ 6
	EventShowType            m_emEventShowType;

	int32_t                  m_iLastRowTimeStamp;
	int32_t                  m_iRowTimeStamp;
	
	int32_t                  m_iLastRow;
	int32_t                  m_iCurrentRow;
	uint32_t                 m_uiRowCount;

	int                      m_iFpnCalculationTimes;
	uint32_t                 m_uiEventFrameTime;

	bool                     m_bIsGeneratingFPN;
	bool                     m_bLoopModeEnabled;
	int						 m_iRotateType;	//rotate param

	vector<EventData>        m_vecEventData;

	int                      m_iMIPIDataFormat;
	uint16_t                 m_uiEventTUnitList[16];
	
	std::ofstream            m_ofFullPic;
	std::ofstream            m_ofLogFile;
	uint64_t                 m_uiEventFrameNo;
};

#endif // CELEX5DATAPROCESSOR_H
