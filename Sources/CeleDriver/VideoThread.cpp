#include "VideoThread.h"
#include <stdio.h>


CVideoThread::CVideoThread()
{
	bRunning = false;
}

CVideoThread::~CVideoThread()
{
}

//Thread function
#ifdef __linux__
void* CVideoThread::VideoThread(void* args)
#else
DWORD  CVideoThread::VideoThread(LPVOID args)
#endif // __linux__
{
	CVideoThread* pThread = (CVideoThread*)args;
	pThread->Worker();
#ifdef __linux__
	return NULL;
#else
	return 0;
#endif // __linux__
}
<<<<<<< HEAD

#ifndef __linux__
unsigned int CVideoThread::staticThreadFunc(void* args)
{
	CVideoThread* pThread = (CVideoThread*)args;
	pThread->Worker();
	return 0;
}
#endif

=======
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
void CVideoThread::Worker(void)
{
    bRunning = true;
    while (bRunning)
    {
        if ( Run() == false )
            break;
    }
    printf("exit thread!\r\n");
}

bool CVideoThread::video_start(void)
{
#ifdef __linux__
    if (pthread_create(&m_threadID, nullptr, VideoThread, this) == 0)
    {
#else
<<<<<<< HEAD
	//m_handle = CreateThread(nullptr, 0, VideoThread, this, 0, NULL);
	m_handle = (HANDLE)_beginthreadex(nullptr, 0, staticThreadFunc, this, 0, nullptr);
=======
	m_handle = CreateThread(nullptr, 0, VideoThread, this, 0, NULL);
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
    if (m_handle)
    {
#endif
        return true;
    }
    return false;
}

void CVideoThread::video_stop(void)
{
    bRunning = false;
#ifdef __linux__
    //pthread_join(m_threadID,nullptr);
#else
    if(m_handle )
    {
        WaitForSingleObject(m_handle, INFINITE);
        CloseHandle(m_handle);
        m_handle = nullptr;
    }
#endif

    printf("stop thread!\r\n");
}
