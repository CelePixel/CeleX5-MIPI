/*
* Copyright (c) 2017-2020 CelePixel Technology Co. Ltd. All Rights Reserved
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

#ifndef CPACKAGE_H
#define CPACKAGE_H

#include <stdint.h>
#include <vector>
#include "celedriver.h"

typedef enum
{
	BUFFER_STATUS_EMPTY = 0,
	BUFFER_STATUS_FULL,
}BUFFER_STATUS;

#define  MAX_ELEMENT_BUFFER_SIZE   43008 // 42976 / 24576 / 24544 / 32752
#define  NOT_USE_VECTOR

class CElement
{
public:
	CElement();
	~CElement();

public:
	uint8_t *begin();
	uint8_t *end();
	void save(uint8_t *buffer, uint16_t wLen);

private:
	uint8_t data[MAX_ELEMENT_BUFFER_SIZE];
	uint16_t wdataLen;
};

class CPackage
{
public:
	CPackage();
	~CPackage();

public:
	void insert(uint8_t *buffer, uint16_t wLen);
	bool getImage(std::vector<uint8_t> &image);
	bool getData(uint8_t* pData, uint32_t& length);
	void end();
	void clearData();
	int  size();

	std::time_t                m_lTimestampEnd;
	std::vector<IMURawData>    m_vecIMUData;

private:
	std::vector<CElement *>    m_vecElementList;
	BUFFER_STATUS              m_emStatus;
	size_t                     m_uiOffset;
	//
	uint8_t*                   m_pPackageBuffer;
};

#endif // CPACKAGE_H