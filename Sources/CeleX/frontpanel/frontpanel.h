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

#ifndef FRONTPANEL_H
#define FRONTPANEL_H

#ifdef _WIN32
#include <windows.h>
#else
#include<unistd.h>
#endif
#include <iostream>
#include <stdint.h>
#include "okFrontPanelDLL.h"
#include "../base/xbase.h"

class FrontPanel
{
private:
    FrontPanel() : mReady(false)
    {
        myxem = new okCFrontPanel;              // xem
        mypll = new okCPLL22393;                // pll22393
    }

public:
    static FrontPanel* getInstance()
    {
        if (!spFrontPanel)
            spFrontPanel = new FrontPanel;
        return spFrontPanel;
    }

    bool initializeFPGA(const std::string& bitfileName);
    void uninitializeFPGA();
    bool isReady() { return mReady; }

    int  wireIn(uint32_t address, uint32_t value, uint32_t mask);
    void wireOut(uint32_t address, uint32_t mask, uint32_t* pValue);
    long blockPipeOut(uint32_t address, int blockSize, long length, unsigned char *data);

    void wait(int ms)
    {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(1000*ms);
#endif
    }

public:

private:
    bool               mReady;
    static FrontPanel* spFrontPanel;
    okCFrontPanel*     myxem;
    okCPLL22393*       mypll;
};

#endif // FRONTPANEL_H
