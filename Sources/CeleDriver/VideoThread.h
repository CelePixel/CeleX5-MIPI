#ifndef __VIDEO_THREAD_H__
#define __VIDEO_THREAD_H__

#ifdef __linux__
#include <pthread.h>
#else
#include <windows.h>
#include <process.h>
#endif
#include "BulkTransfer.h"

class CVideoThread
{
public:
	CVideoThread();
	~CVideoThread();
public:
	bool video_start(void);
	void video_stop(void);
protected:
    virtual bool Run(void) = 0;
private:
	//Thread function
#ifdef __linux__
	static void* VideoThread(void* args);
#else
	static DWORD VideoThread(LPVOID args);
#endif // __linux__
	void Worker(void);
private:
	bool bRunning = false;

#ifdef __linux__
	pthread_t m_threadID;
#else
	HANDLE m_handle;
#endif // __linux__
};

#endif // __VIDEO_THREAD_H__
