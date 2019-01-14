#include "dataqueue.h"
#include <iostream>
#include <cstring>

#define PIXELS_NUMBER    1024000

DataQueue::DataQueue() : m_size(0)
{
}

DataQueue::~DataQueue()
{
}

void DataQueue::push(unsigned char *pData, long length)
{
    DataInfo dataIn;
    dataIn.pData = pData;
    dataIn.length = length;
    m_queue.push(dataIn);
    m_size += dataIn.length;
}

void DataQueue::pop(unsigned char *&pData, long *length)
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
    }
}

uint32_t DataQueue::size()
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
