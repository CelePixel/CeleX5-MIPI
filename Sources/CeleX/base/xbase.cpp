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

#include "xbase.h"
#include <fstream>
#include <time.h>
#include <iostream>
#include <sstream>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

XBase::XBase()
{

}

XBase::~XBase()
{

}

std::string XBase::getApplicationDirPath()
{
    //D:\\Work\\build-okBaseTest-Desktop_Qt_5_9_1_MinGW_32bit-Debug\\debug\\okBaseTest.exe
#ifdef _WIN32
    char path[1024];
    memset(path, 0, 1024);
    GetModuleFileNameA(NULL, path, 1024);
    std::string strPath = path;
    int len = strPath.find_last_of('\\');
    return strPath.substr(0, len);
#else
    char path[1024];
    int cnt = readlink("/proc/self/exe", path, 1024);
	cout << "XBase::getApplicationDirPath: readlink count = " << cnt << endl;
    if(cnt < 0|| cnt >= 1024)
    {
        return NULL;
    }
    for(int i = cnt; i >= 0; --i)
    {
        if(path[i]=='/')
        {
            path[i + 1]='\0';
            break;
        }
    }
    string strPath(path);
    return strPath;
#endif
}

string XBase::getDateTimeStamp()
{
    time_t tt = time(NULL);
    tm* t = NULL;
#ifdef _WIN32
    localtime_s(t, &tt);
#else
    t = localtime(&tt);
#endif
    std::stringstream ss;
    //year
    ss << (t->tm_year+1900);
    //month
    if (t->tm_mon < 10)
        ss << "0" << t->tm_mon+1;
    else
        ss << t->tm_mon+1;
    //
    //day
    if (t->tm_mday < 10)
        ss << "0" << t->tm_mday;
    else
        ss << t->tm_mday;
    //
    ss << "_";
    //hour
    if (t->tm_hour < 10)
        ss << "0" << t->tm_hour;
    else
        ss << t->tm_hour;
    //Minute
    if (t->tm_min < 10)
        ss << "0" << t->tm_min;
    else
        ss << t->tm_min;
    //Second
    if (t->tm_sec < 10)
        ss << "0" << t->tm_sec;
    else
        ss << t->tm_sec;
    return string(ss.str());
}

int XBase::getTimeStamp()
{
#ifdef _WIN32
	time_t tt = time(NULL);
	tm t;
	localtime_s(&t, &tt);

	return 3600 * t.tm_hour + 60 * t.tm_min + t.tm_sec;
#else
	time_t tt = time(NULL);
	struct tm* t;
	t = localtime(&tt);
	return 3600 * t->tm_hour + 60 * t->tm_min + t->tm_sec;
#endif
}

bool XBase::isFileExists(std::string filePath)
{
    //"D:/Work/build-okBaseTest-Desktop_Qt_5_9_1_MinGW_32bit-Debug/debug/top.bit";
    printf("XBase::isFileExists: %s", filePath.c_str());
    fstream _file;
    _file.open(filePath.c_str(), ios::in);
    if (!_file)
    {
        printf("%s can't find!", filePath.c_str());
        return false;
    }
    _file.close();
    return true;
}
