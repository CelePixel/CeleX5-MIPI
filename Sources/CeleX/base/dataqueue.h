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

#ifndef DATAQUEUE_H
#define DATAQUEUE_H

#include <queue>
#include <stdint.h>
#include <array>
#include <vector>
#include "../include/celextypes.h"

#define ARRAY_SIZE     12
#define MAX_DATA_LEN   1536001

typedef struct DataInfo
{
	uint8_t*           pData;
	unsigned long      length;
	time_t             timestamp;
	std::vector<IMURawData> vecIMUData;
} DataInfo;

/* 
* This class provides a data queue for playing back recorded bin file.
*/
class DataQueue
{
public:
	DataQueue();
	~DataQueue();

	bool push(uint8_t* pData, uint32_t length, std::time_t timestamp = 0);
	bool push(uint8_t* pData, uint32_t length, std::vector<IMURawData> &imuData, std::time_t timestamp);

	bool pop(uint8_t* pData, uint32_t* length, std::time_t* timestamp);
	bool pop(uint8_t* pData, uint32_t* length, std::vector<IMURawData> &imuData, std::time_t* timestamp);

	uint32_t size();
	void clear();

private:
	std::array<DataInfo, ARRAY_SIZE> m_arrayData;
	uint32_t             m_uiSize;
	int                  m_iHead;
	int                  m_iTail;
};

#endif // DATAQUEUE_H
