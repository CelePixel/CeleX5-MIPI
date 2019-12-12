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

#include <iostream>
#include <cstring>
#include "dataqueue.h"
#include "../include/celextypes.h"

DataQueue::DataQueue()
	: m_uiSize(0)
	, m_iHead(0)
	, m_iTail(0)
{
	for (int i = 0; i < m_arrayData.size(); i++)
	{
		m_arrayData[i].length = MAX_DATA_LEN;
		m_arrayData[i].pData = new uint8_t[MAX_DATA_LEN];
	}
}

DataQueue::~DataQueue()
{
	for (int i = 0; i < m_arrayData.size(); i++)
	{
		delete[] m_arrayData[i].pData;
		m_arrayData[i].pData = nullptr;
	}
}

/*
*  @function:  push
*  @brief   :  add a packet data at the end of the data array (m_arrayData)
*  @input   :  pData:     a packet of sensor raw data
*              length:    the size of the packet pData
*              timestamp: the pc timestamp when the packet pData was received
*  @output  :
*  @return  :
*/
bool DataQueue::push(uint8_t* pData, uint32_t length, std::time_t timestamp)
{
	if (nullptr == pData)
	{
		return false;
	}
	if (m_uiSize == ARRAY_SIZE)
	{
		std::cout << __FUNCTION__ << ": data array is full!" << std::endl;
		return false;
	}
	int lenCopy = length > MAX_DATA_LEN ? MAX_DATA_LEN : length;
	memcpy(m_arrayData[m_iTail].pData, pData, lenCopy);
	m_arrayData[m_iTail].length = lenCopy;
	m_arrayData[m_iTail].timestamp = timestamp;
	m_uiSize++;
	m_iTail++;
	if (m_iTail == ARRAY_SIZE)
		m_iTail = 0;
	return true;
}

/*
*  @function:  push
*  @brief   :  add a packet data at the end of the data array (m_arrayData)
*  @input   :  pData:     a packet of sensor raw data
*              length:    the size of the packet pData
*              imuData:   a vector of imu raw data
*              timestamp: the pc timestamp when the packet pData was received
*  @output  :
*  @return  :
*/
bool DataQueue::push(uint8_t* pData, uint32_t length, std::vector<IMURawData> &imuData, std::time_t timestamp)
{
	if (nullptr == pData)
	{
		return false;
	}
	if (m_uiSize == ARRAY_SIZE)
	{
		std::cout << __FUNCTION__ << ": data array is full!" << std::endl;
		return false;
	}
	int lenCopy = length > MAX_DATA_LEN ? MAX_DATA_LEN : length;
	memcpy(m_arrayData[m_iTail].pData, pData, lenCopy);
	m_arrayData[m_iTail].length = lenCopy;
	m_arrayData[m_iTail].timestamp = timestamp;
	m_arrayData[m_iTail].vecIMUData = imuData;
	m_uiSize++;
	m_iTail++;
	if (m_iTail == ARRAY_SIZE)
		m_iTail = 0;
	return true;
}

/*
*  @function:  pop
*  @brief   :  pop the head packet data in data array (m_arrayData)
*  @input   :  
*  @output  :  pData:     a packet of sensor raw data
*              length:    the size of the packet pData
*              timestamp: the pc timestamp when the packet pData was received
*  @return  :
*/
bool DataQueue::pop(uint8_t* pData, uint32_t* length, std::time_t* timestamp)
{
	if (nullptr == pData)
	{
		return false;
	}
	if (m_uiSize == 0)
	{
		std::cout << __FUNCTION__ << ": data array is empty!" << std::endl;
		return false;
	}
	if (nullptr == m_arrayData[m_iHead].pData)
	{
		//pData = NULL;
		*length = 0;
	}
	else
	{
		//pData = m_arrayData[m_iHead].pData;	
		memcpy(pData, m_arrayData[m_iHead].pData, m_arrayData[m_iHead].length);
		*length = m_arrayData[m_iHead].length;
		*timestamp = m_arrayData[m_iHead].timestamp;
	}
	m_uiSize--;
	m_iHead++;
	if (ARRAY_SIZE == m_iHead)
		m_iHead = 0;
	return true;
}

/*
*  @function:  pop
*  @brief   :  pop the head packet data in data array (m_arrayData)
*  @input   :
*  @output  :  pData:     a packet of sensor raw data
*              length:    the size of the packet pData
*              imuData:   a vector of imu raw data
*              timestamp: the pc timestamp when the packet pData was received
*  @return  :
*/
bool DataQueue::pop(uint8_t* pData, uint32_t* length, std::vector<IMURawData>& imuData, std::time_t* timestamp)
{
	if (nullptr == pData)
	{
		return false;
	}
	if (m_uiSize == 0)
	{
		std::cout << __FUNCTION__ << ": data array is empty!" << std::endl;
		return false;
	}
	if (nullptr == m_arrayData[m_iHead].pData)
	{
		//pData = NULL;
		*length = 0;
	}
	else
	{
		//pData = m_arrayData[m_iHead].pData;	
		memcpy(pData, m_arrayData[m_iHead].pData, m_arrayData[m_iHead].length);
		*length = m_arrayData[m_iHead].length;
		*timestamp = m_arrayData[m_iHead].timestamp;
		imuData.swap(m_arrayData[m_iHead].vecIMUData);
	}
	m_uiSize--;
	m_iHead++;
	if (ARRAY_SIZE == m_iHead)
		m_iHead = 0;
	return true;
}

/*
*  @function:  size
*  @brief   :  get the size of the data array (m_arrayData)
*  @input   : 
*  @output  :
*  @return  : return the size of the data array (m_arrayData)
*/
uint32_t DataQueue::size()
{
	return m_uiSize;
}

/*
*  @function:  clear
*  @brief   :  reset the head / tail index, and data array (m_arrayData) size
*  @input   :  
*  @output  :
*  @return  :
*/
void DataQueue::clear()
{
	m_uiSize = 0;
	m_iHead = 0;
	m_iTail = 0;
}
