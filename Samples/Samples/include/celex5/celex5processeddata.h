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

#ifndef CELEX5_PROCESSED_DATA_H
#define CELEX5_PROCESSED_DATA_H

#include "celex5.h"

/*
*  This class is used to get the sensor data includes image 
*  buffer, raw event data and some setting parameters.
*/
class CeleX5ProcessedData
{
public:
	CeleX5ProcessedData();
	~CeleX5ProcessedData();
	inline CeleX5::CeleX5Mode getSensorMode() { return m_emSensorMode; }
	inline void setSensorMode(CeleX5::CeleX5Mode mode) { m_emSensorMode = mode; }
	inline int getLoopNum() { return m_iLoopNum; }
	inline void setLoopNum(int loopNum) { m_iLoopNum = loopNum; }
	inline void setTemperature(uint16_t temperature) { m_uiTemperature = temperature; }
	inline uint16_t getTemperature() { return m_uiTemperature; }
	inline void setFullFrameFPS(uint16_t fps) { m_uiFullFrameFPS = fps; }
	inline uint16_t getFullFrameFPS() { return m_uiFullFrameFPS; }
	inline void updateFPNProgress(int value) { m_iFPNProgressValue = value; }
	inline int  getFPNProgress() { return m_iFPNProgressValue; }

private:
	CeleX5::CeleX5Mode     m_emSensorMode;
	int                    m_iLoopNum;
	uint16_t               m_uiTemperature;
	uint16_t               m_uiFullFrameFPS;
	int                    m_iFPNProgressValue;
};

#endif // CELEX5_PROCESSED_DATA_H
