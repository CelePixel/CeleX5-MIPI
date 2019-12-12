/*
* Copyright (c) 2017-2020 CelePixel Technology Co. Ltd. All Rights Reserved
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

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <fstream>
#include <time.h>
#include <iostream>
#include <sstream>
#include <cstring>
#include "filedirectory.h"

FileDirectory::FileDirectory()
{

}

FileDirectory::~FileDirectory()
{

}

/*
*  @function:  getApplicationDirPath
*  @brief   :  get the directory where exe is located
*  @input   :  
*  @output  :
*  @return  :  the directory where exe is located
*/
std::string FileDirectory::getApplicationDirPath()
{
	char path[1024];
    memset(path, 0, 1024);
#ifdef _WIN32
    GetModuleFileNameA(nullptr, path, 1024);
    std::string strPath = path;
    int len = strPath.find_last_of('\\');
    return strPath.substr(0, len);
#else
    int cnt = readlink("/proc/self/exe", path, 1024);
	std::cout << "FileDirectory::getApplicationDirPath: readlink count = " << cnt << std::endl;
    if (cnt < 0 || cnt >= 1024)
    {
		return std::string("");
    }
    for (int i = cnt; i >= 0; --i)
    {
        if (path[i] == '/')
        {
            path[i + 1] = '\0';
            break;
        }
    }
    return std::string(path);
#endif
}

/*
*  @function:  isFileExists
*  @brief   :  check if the given file exists 
*  @input   :  filePath: a string of a file path and name
*  @output  :
*  @return  :  true means the file exist, otherwise it doesn't exist
*/
bool FileDirectory::isFileExists(std::string filePath)
{
    //printf("FileDirectory::isFileExists: %s", filePath.c_str());
    std::fstream fsFile;
	fsFile.open(filePath.c_str(), std::ios::in);
    if (!fsFile.is_open())
    {
        printf("%s can't find!\n", filePath.c_str());
        return false;
    }
	fsFile.close();
    return true;
}
