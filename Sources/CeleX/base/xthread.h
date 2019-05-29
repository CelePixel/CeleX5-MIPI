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

#ifndef XTHREAD_H
#define XTHREAD_H

#include <string>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

class XThread
{
public:
    XThread(const std::string threadName = "newthread");
    virtual ~XThread();

    virtual void run() = 0;
    virtual bool start(bool bSuspended = false);

    void join(int timeout = -1);
    void resume();
    void suspend();
    bool terminate();
    bool isRunning();

    unsigned int getThreadID();
    std::string getThreadName();
    void setThreadName(std::string threadName);

private:
    bool createThread(bool bSuspended = false);
#ifdef _WIN32
    static unsigned int WINAPI staticThreadFunc(void* args);
#else
    static void* staticThreadFunc(void* args);
#endif

protected:
    std::string           m_threadName;
    volatile bool         m_bRun;
    bool                  m_bSuspended;

#ifdef _WIN32
    HANDLE                m_handle;
    unsigned int          m_threadID;
#else
    pthread_t             m_threadID;
    pthread_mutex_t       m_mutex;
    pthread_cond_t        m_cond;
#endif 
};

#endif // XTHREAD_H
