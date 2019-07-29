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

#ifndef HHSEQUENCEMGR_H
#define HHSEQUENCEMGR_H

#include <string>
#include <vector>
#include <map>
#include <stdint.h>

#include "../include/celex5/celex5.h"

using namespace std;

class HHCommandBase;

class HHSequence
{
public:
    HHSequence(const std::string& name);
    virtual ~HHSequence();

    std::string name();
    bool isShown();
    void setShow(bool bShow);
    bool isAdvanced();
    void setAdvanced(bool bAdvanced);
    void addCommand(HHCommandBase* pCmd);
    void setNext(const std::string& nextName);
    std::string getNext();
    virtual bool fire();

private:
    std::string mName;
    std::string mNext;
    bool mShow;
    bool mAdvanced;

protected:
    std::vector<HHCommandBase*> mCommands;
};

class HHSequenceSlider : public HHSequence
{
public:
    HHSequenceSlider(const std::string& name, uint32_t min, uint32_t max, uint32_t step);

    uint32_t getMax();
    uint32_t getMin();
    uint32_t getValue();
    uint32_t getStep();
    void setValue(uint32_t value);

    bool fireWithArg(uint32_t newValue);

private:
    uint32_t mMin;
    uint32_t mMax;
    uint32_t mValue;
    uint32_t mStep;
};

class HHSequenceMgr
{
public:
    HHSequenceMgr();
    ~HHSequenceMgr();

    //--- for CeleX5 ---
    bool parseCeleX5Cfg(const string& cfgName);
    bool saveCeleX5XML(map<string, vector<CeleX5::CfgInfo>> mapCfgInfo);
<<<<<<< HEAD
=======
	bool saveCeleX5XML(const string& cfgName);
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
	map<string, vector<HHCommandBase*>> getCeleX5Cfg();

private:
	map<string, HHCommandBase*>		      m_CommandMap_CeleX5;
	map<string, vector<HHCommandBase*>>   m_CommandList_CeleX5;
};

#endif // HHSEQUENCEMGR_H
