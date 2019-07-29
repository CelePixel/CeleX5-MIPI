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

#ifndef HHDATAQUEUE_H
#define HHDATAQUEUE_H

#include <queue>
#include <stdint.h>
#include <array>
<<<<<<< HEAD

typedef struct DataInfo
{
    unsigned char* pData;
    unsigned long  length;
	time_t	timeStamp;
=======
#include <vector>
#include "../include/celextypes.h"

typedef struct DataInfo
{
    unsigned char*     pData;
    unsigned long      length;
	time_t             timeStamp;
	std::vector<IMURawData> vecIMUData;
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
} DataInfo;

class DataQueue
{
public:
    DataQueue();
    ~DataQueue();

    void push(unsigned char* pData, long length,time_t timeStamp = 0);
    void pop(unsigned char*& pData, long* length, time_t* timeStamp);
	unsigned long size();
    void clear();

private:
    std::queue<DataInfo> m_queue;
    unsigned long        m_size;
};

#define ARRAY_SIZE     12
#define MAX_DATA_LEN   1536001
class DataQueueEx
{
public:
	DataQueueEx();
	~DataQueueEx();

	void push(unsigned char* pData, long length, time_t timeStamp = 0);
<<<<<<< HEAD
	void pop(unsigned char* pData, long* length, time_t* timeStamp);
=======
	void push(unsigned char* pData, long length, std::vector<IMURawData> imuData, time_t timeStamp);

	void pop(unsigned char* pData, long* length, time_t* timeStamp);
	void pop(unsigned char* pData, long* length, std::vector<IMURawData> &imuData, time_t* timeStamp);
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
	unsigned long size();
	void clear();

private:
	std::queue<DataInfo> m_queue;
	std::array<DataInfo, ARRAY_SIZE> m_arrayData;
	unsigned long        m_size;
	int                  m_iHead;
	int                  m_iTail;
};

class CirDataQueue
{
public:
    CirDataQueue(int queueCapacity);
    ~CirDataQueue();

    int getLength(); //get the length of the queue
    int getCapacity();

    bool enqueue(unsigned char* pData);  //push a element
    bool dequeue(unsigned char*& pData); //pop a element
    bool isEmpty();
    bool isFull();
    void clear();
    unsigned char* head();

private:
    std::vector<DataInfo>  m_queue;
    int                    m_iHead;
    int                    m_iTail;
    int                    m_iQueueLenth;
    int                    m_iQueueCapacity;
};

class CirDataQueueEx
{
public:
	CirDataQueueEx();
	~CirDataQueueEx();

	void allocMemory(uint64_t capacity);
	void clearMemery();
	int size(); //get the length of the queue
	int capacity();

	bool push(unsigned char* pData, uint32_t length);  //push a element
	bool pop(unsigned char* pData, uint32_t* length); //pop a element
	bool isEmpty();
	bool isFull();
	void clear();
	unsigned char* head();

private:
	std::queue<uint32_t>   m_queueSize;
	unsigned char*         m_pBuffer;
	int                    m_iHead;
	int                    m_iTail;
	int                    m_iLength;
	uint64_t               m_iCapacity;
};

#endif // HHDATAQUEUE_H
