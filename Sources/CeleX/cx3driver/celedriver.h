/*
* Copyright (c) 2017-2020  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef CELEPIXEL_H
#define CELEPIXEL_H

#include <stdint.h>
#include <vector>
#include <ctime>
#include <string>
#include "../include/celextypes.h"

class Cypress;
class CeleDriver
{
public:
	CeleDriver(void);
	~CeleDriver(void);

public:
	bool openUSB();
	bool openStream();

	void closeUSB();
	void closeStream(); 

	bool writeSerialNumber(std::string number); 
	std::string getSerialNumber();
	std::string getFirmwareVersion(); 
	std::string getFirmwareDate();

	bool getSensorData(uint8_t* pData, uint32_t& length);
	bool getSensorData(uint8_t* pData, uint32_t& length, std::time_t& timestampEnd, std::vector<IMURawData>& imuData); //added by xiaoqin @2019.01.24
	void clearData();

	uint16_t getALSValue();

public:
	bool i2cSet(uint16_t reg, uint16_t value);
	bool i2cGet(uint16_t reg, uint16_t &value);
	bool mipiSet(uint16_t reg, uint16_t value);
	bool mipiGet(uint16_t reg, uint16_t &value);

private:
	Cypress*    m_pCypress;
};

#endif // CELEPIXEL_H
