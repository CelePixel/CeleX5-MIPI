/*
* Copyright (c) 2017-2018 CelePixel Technology Co. Ltd. All Rights Reserved
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
#include "xthread.h"

XThread::XThread(const std::string threadName)
    : m_threadName(threadName)
    , m_threadID(0)
    , m_bRun(false)
    , m_bSuspended(false)
#ifndef _WIN32
    , m_mutex(PTHREAD_MUTEX_INITIALIZER)
    , m_cond(PTHREAD_COND_INITIALIZER)
#endif
{

}

XThread::~XThread()
{

}

//Create a thread and run (default) or hang
bool XThread::start(bool bSuspend/* = false*/)
{
    m_bRun = createThread(bSuspend);
    return m_bRun;
}

//Create a thread and run (default) or hang
bool XThread::createThread(bool bSuspend/* = false*/)
{
    if(!m_bRun)
    {
#ifdef _WIN32
        if(bSuspend)
        {
            m_handle = (HANDLE)_beginthreadex(NULL, 0, staticThreadFunc, this, CREATE_SUSPENDED, &m_threadID);
            m_bSuspended = true;
        }
        else
        {
            m_handle = (HANDLE)_beginthreadex(NULL, 0, staticThreadFunc, this, 0, &m_threadID);
        }
        m_bRun = (NULL != m_handle);
#else
        int status = pthread_create(&m_threadID, NULL, staticThreadFunc, this);
        if (status != 0)
            std::cout << "creating thread failure" << std::endl;
        else
            m_bRun = true;
#endif
    }
    return m_bRun;
}

//If the waiting time (milliseconds) is negative, it means unlimited wait.
void XThread::join(int timeout/* = -1*/)
{
#ifdef _WIN32
    if(m_handle && m_bRun)
    {
        if(timeout < 0)
            timeout = INFINITE;
        ::WaitForSingleObject(m_handle, timeout);
    }
#else
#endif
}

//Resume the suspended thread
void XThread::resume()
{
	std::cout << "XThread::resume: m_bRun = " << m_bRun << ", m_bSuspended = " << m_bSuspended << std::endl;
    if (m_bRun && m_bSuspended)
    {
        std::cout << "XThread::resume" << std::endl;
#ifdef _WIN32
        ::ResumeThread(m_handle);
        m_bSuspended = false;
#else
        pthread_mutex_lock(&m_mutex);
        m_bSuspended = false;
        pthread_cond_signal(&m_cond);
        pthread_mutex_unlock(&m_mutex);
#endif
    }
}

//Suspend the thread
void XThread::suspend()
{
    if (m_bRun && !m_bSuspended)
    {
        std::cout << "XThread::suspend" << std::endl;
#ifdef _WIN32
		m_bSuspended = true;
        ::SuspendThread(m_handle);
#else
        pthread_mutex_lock(&m_mutex);
        m_bSuspended = true;
        pthread_mutex_unlock(&m_mutex);
#endif
    }
}

//Terminate the thread
bool XThread::terminate()
{
#ifdef _WIN32
    unsigned long exitCode = 0;
    if(m_handle && m_bRun)
    {
        if (::TerminateThread(m_handle, exitCode))
        {
            ::CloseHandle(m_handle);
            m_handle = NULL;
            m_bRun = false;
            return true;
        }
        printf("XThread::terminate: exitCode = %ld\n", exitCode);
    }
#else
#endif
    return false;
}

bool XThread::isRunning()
{
    return m_bRun;
}

unsigned int XThread::getThreadID()
{
    return m_threadID;
}

std::string XThread::getThreadName()
{
    return m_threadName;
}

void XThread::setThreadName(std::string threadName)
{
    m_threadName = threadName;
}

#ifdef _WIN32
//Thread function
unsigned int XThread::staticThreadFunc(void* arg)
{
    XThread* pThread = (XThread*)arg;  //Get the thread class pointer
    pThread->run();

    return 0;
}
#else
//Thread function
void* XThread::staticThreadFunc(void* args)
{
    XThread* pThread = static_cast<XThread *>(args); //Get the thread class pointer
    if (pThread)
        pThread->run();
    return NULL;
}
#endif
