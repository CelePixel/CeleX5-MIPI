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

#include "dataqueue.h"
#include <iostream>
#include <cstring>
#include "../include/celextypes.h"

using namespace std;

DataQueue::DataQueue() : m_size(0)
{
}

DataQueue::~DataQueue()
{
}

void DataQueue::push(unsigned char *pData, long length,time_t timeStamp)
{
    DataInfo dataIn;
    unsigned char* pBuffer = new unsigned char[length];
    memcpy(pBuffer, pData, length);
    dataIn.pData = pBuffer;
    dataIn.length = length;
	dataIn.timeStamp = timeStamp;
    m_queue.push(dataIn);
    m_size += dataIn.length;
}

void DataQueue::pop(unsigned char *&pData, long *length, time_t* timeStamp)
{
    if (m_queue.size() <=0)
        return;

    DataInfo dataOut = m_queue.front();
    m_queue.pop();
    m_size -= dataOut.length;

    if (!dataOut.pData)
    {
        *length = 0;
    }
    else
    {
        pData = dataOut.pData;
        *length = dataOut.length;
		*timeStamp = dataOut.timeStamp;
    }
}

unsigned long DataQueue::size()
{
    return m_size;
}

void DataQueue::clear()
{
    while (m_queue.size() > 0)
    {
        DataInfo dataToDelete;
        dataToDelete = m_queue.front();
        delete [] dataToDelete.pData;
        m_queue.pop();
    }
    m_size = 0;
}

//----------------------------------------
//-------------- DataQueueEx -------------
DataQueueEx::DataQueueEx()
	: m_size(0)
	, m_iHead(0)
	, m_iTail(0)
{
	for (int i = 0; i < m_arrayData.size(); i++)
	{
		m_arrayData[i].length = MAX_DATA_LEN;
		m_arrayData[i].pData = new unsigned char[MAX_DATA_LEN];
	}
}

DataQueueEx::~DataQueueEx()
{
}

void DataQueueEx::push(unsigned char *pData, long length, time_t timeStamp)
{
	if (m_size == ARRAY_SIZE)
	{
		cout << __FUNCTION__ << ": data array is full!" << endl;
		return;
	}
	int len_copy = length > MAX_DATA_LEN ? MAX_DATA_LEN : length;
	memcpy(m_arrayData[m_iTail].pData, pData, len_copy);
	m_arrayData[m_iTail].length = len_copy;
	m_arrayData[m_iTail].timeStamp = timeStamp;
	m_size++;
	m_iTail++;
	if (m_iTail == ARRAY_SIZE)
		m_iTail = 0;
}

void DataQueueEx::pop(unsigned char *pData, long *length, time_t* timeStamp)
{
	if (m_size <= 0)
	{
		cout << __FUNCTION__ << ": data array is empty!" << endl;
		return;
	}
	if (NULL == m_arrayData[m_iHead].pData)
	{
		//pData = NULL;
		*length = 0;
	}
	else
	{
		//pData = m_arrayData[m_iHead].pData;	
		memcpy(pData, m_arrayData[m_iHead].pData, m_arrayData[m_iHead].length);
		*length = m_arrayData[m_iHead].length;
		*timeStamp = m_arrayData[m_iHead].timeStamp;
	}
	m_size--;
	m_iHead++;
	if (ARRAY_SIZE == m_iHead)
		m_iHead = 0;
}

unsigned long DataQueueEx::size()
{
	return m_size;
}

void DataQueueEx::clear()
{
	m_size = 0;
	m_iHead = 0;
	m_iTail = 0;
}

//----------------------------------------
//------------- CirDataQueue -------------
CirDataQueue::CirDataQueue(int queueCapacity)
    : m_iHead(0)
    , m_iTail(0)
    , m_iQueueLenth(0)
    , m_iQueueCapacity(queueCapacity)
{
    for (int i = 0; i < queueCapacity; ++i)
    {
        DataInfo dataIn;
        dataIn.pData = new unsigned char[PIXELS_NUMBER];
        for (int i = 0; i < PIXELS_NUMBER; ++i)
            dataIn.pData[i] = 0;
        dataIn.length = 0;
        m_queue.push_back(dataIn);
    }
}

CirDataQueue::~CirDataQueue()
{
    m_queue.clear();
}

int CirDataQueue::getLength()
{
    return m_iQueueLenth;
}

int CirDataQueue::getCapacity()
{
    return m_iQueueCapacity;
}

bool CirDataQueue::enqueue(unsigned char *pData)
{
	if (0 == m_iQueueCapacity)
		return false;

    if (isFull())
    {
        std::cout << "CirDataQueue::enqueue: queue is full!";
        return false;
    }
    else
    {
        memcpy(m_queue[m_iTail].pData, pData, PIXELS_NUMBER);
        ++m_iTail;
        m_iTail = m_iTail % m_iQueueCapacity;
        ++m_iQueueLenth;

        //std::cout << "CirDataQueue::enqueue: length = " << m_iQueueLenth << std::endl;
        return true;
    }
}

bool CirDataQueue::dequeue(unsigned char *&pData)
{
    if (isEmpty())
    {
        return false;
    }
    else
    {
        DataInfo dataOut = m_queue[m_iHead];
        m_iHead++;
        m_iHead = m_iHead % m_iQueueCapacity;
        --m_iQueueLenth;
        pData = dataOut.pData;

        //std::cout << "--- CirDataQueue::dequeue: length = " << m_iQueueLenth << std::endl;
        return true;
    }
}

bool CirDataQueue::isEmpty()
{
    if (0 == m_iQueueLenth)
    {
        return true;
    }
    return false;
}

bool CirDataQueue::isFull()
{
    if (m_iQueueLenth == m_iQueueCapacity)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CirDataQueue::clear()
{
    std::cout << "CirDataQueue::clear" << std::endl;
    m_iHead = 0;
    m_iTail = 0;
    m_iQueueLenth = 0;
}

unsigned char *CirDataQueue::head()
{
    if (0 != m_iQueueCapacity)
    {
        return m_queue[m_iTail].pData;
    }
    return NULL;
}


//----------------------------------------
//------------- CirDataQueueEx -------------
CirDataQueueEx::CirDataQueueEx()
	: m_iHead(0)
	, m_iTail(0)
	, m_iLength(0)
	, m_iCapacity(0)
	, m_pBuffer(NULL)
{
}

CirDataQueueEx::~CirDataQueueEx()
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

void CirDataQueueEx::allocMemory(uint64_t capacity)
{
	m_pBuffer = new unsigned char[capacity];
	m_iCapacity = capacity;
}

void CirDataQueueEx::clearMemery()
{
	if (m_pBuffer)
	{
		delete[] m_pBuffer;
		m_pBuffer = NULL;
	}
}

int CirDataQueueEx::size()
{
	return m_iLength;
}

int CirDataQueueEx::capacity()
{
	return m_iCapacity;
}

bool CirDataQueueEx::push(unsigned char* pData, uint32_t length)
{
	if (0 == m_iCapacity)
		return false;

	if (isFull())
	{
		cout << "CirDataQueueEx::push: data buffer is full!" << endl;
		return false;
	}
	if (m_iCapacity - m_iLength >= length)
	{
		uint32_t lenTail2End = m_iCapacity - m_iTail;
		if (lenTail2End >= length)
		{
			memcpy(m_pBuffer + m_iTail, pData, length);
			m_iTail += length;
		}
		else
		{
			if (lenTail2End > 0)
				memcpy(m_pBuffer + m_iTail, pData, lenTail2End);
			memcpy(m_pBuffer, pData + lenTail2End, length - lenTail2End);
			m_iTail = length - lenTail2End;
		}
		m_iLength += length;

		m_queueSize.push(length);

		//cout << "m_iHead = " << m_iHead << endl;
		//cout << "m_iTail = " << m_iTail << endl;
		//cout << "CirDataQueueEx::push: length = " << m_iLength << endl;
	}
	else
	{
		cout << "CirDataQueueEx::push: data buffer is too large to store!" << endl;
		return false;
	}
	return true;
}

bool CirDataQueueEx::pop(unsigned char *pData, uint32_t* length)
{
	if (NULL == pData)
		return false;
	if (isEmpty())
	{
		return false;
	}
	else
	{
		if (m_queueSize.size() > 0)
		{
			uint32_t len = m_queueSize.front();
			//cout << "-------------------- len = " << len << endl;
			if (len <= m_iLength)
			{
				if (m_iTail > m_iHead)
				{
					memcpy(pData, m_pBuffer + m_iHead, len);
					m_iHead += len;
					m_iLength -= len;
				}
				else
				{
					int len_Head_To_End = m_iCapacity - m_iHead;
					//cout << "len_Head_To_End = " << len_Head_To_End << endl;
					if (len <= len_Head_To_End)
					{
						memcpy(pData, m_pBuffer + m_iHead, len);
						m_iHead += len;
						m_iLength -= len;
					}
					else
					{
						if (len_Head_To_End > 0)
						{
							memcpy(pData, m_pBuffer + m_iHead, len_Head_To_End);
						}
						memcpy(pData + len_Head_To_End, m_pBuffer, len - len_Head_To_End);
						m_iHead = len - len_Head_To_End;
						m_iLength -= len;
					}
				}
				*length = len;
				m_queueSize.pop();
				//cout << "m_iHead = " << m_iHead << endl;
				//cout << "m_iTail = " << m_iTail << endl;
				//cout << "CirDataQueueEx::pop: length = " << m_iLength << endl;
			}
			else
			{
				;
			}
			return true;
		}
	}
}

bool CirDataQueueEx::isEmpty()
{
	if (0 == m_iLength)
	{
		return true;
	}
	return false;
}

bool CirDataQueueEx::isFull()
{
	if (m_iLength == m_iCapacity)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CirDataQueueEx::clear()
{
	cout << "CirDataQueueEx::clear" << endl;
	m_iHead = 0;
	m_iTail = 0;
	m_iLength = 0;
}

unsigned char *CirDataQueueEx::head()
{
	if (0 != m_iCapacity)
	{
		return m_pBuffer + m_iHead;
	}
	return NULL;
}
