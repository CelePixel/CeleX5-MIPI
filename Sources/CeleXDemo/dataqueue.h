#ifndef HHDATAQUEUE_H
#define HHDATAQUEUE_H

#include <queue>
#include <stdint.h>

typedef struct DataInfo
{
    unsigned char* pData;
    long           length;
} DataInfo;

class DataQueue
{
public:
    DataQueue();
    ~DataQueue();

    void push(unsigned char* pData, long length);
    void pop(unsigned char*& pData, long* length);
    uint32_t size();
    void clear();

private:
    std::queue<DataInfo> m_queue;
    uint32_t             m_size;
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

#endif // HHDATAQUEUE_H
