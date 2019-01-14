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

#include "hhsequencemgr.h"
#include "hhcommand.h"
#include "hhxmlreader.h"
#include <iostream>
#include "../include/celextypes.h"

#ifdef XML_PARSE_USE_QT
#include <QDomDocument>
#else
#include "tinyxml/tinyxml.h"
#endif

using namespace std;

HHSequenceMgr::HHSequenceMgr()
{
}

HHSequenceMgr::~HHSequenceMgr()
{
    while(mCommandList.size() > 0)
    {
        HHCommandBase* cmd = mCommandList.back();
        mCommandList.pop_back();
        delete cmd;
    }
    while(mSequenceList.size() > 0)
    {
        HHSequence* seq = mSequenceList.back();
        mSequenceList.pop_back();
        delete seq;
    }
}

bool HHSequenceMgr::parseCommandList()
{
    HHXmlReader xml;
#ifdef XML_PARSE_USE_QT
    QDomDocument dom;
#else
    TiXmlDocument dom;
#endif
    if (xml.parse(FILE_COMMANDS, &dom))
    {
        if (xml.importCommands(mCommandList, &dom))
        {
            for(std::vector<HHCommandBase*>::iterator itr = mCommandList.begin(); itr != mCommandList.end(); itr++)
            {
                string name =(*itr)->name();
                //cout << "HHSequenceMgr::parseCommandList: " << name << endl;
                map<std::string, HHCommandBase*>::iterator it = mCommandMap.find(name);
                if (it == mCommandMap.end())
                {
                    mCommandMap[name] = *itr;
                }
                else
                {
                    cout << "More than one command has the same name: " << name << endl;
                }
            }
            return true;
        }
    }
    return false;
}

bool HHSequenceMgr::parseSequenceList()
{
    HHXmlReader xml;
#ifdef XML_PARSE_USE_QT
    QDomDocument dom;
#else
    TiXmlDocument dom;
#endif
    if (xml.parse(FILE_SEQUENCES, &dom))
    {
        if (xml.importSequences(this, mSequenceList, &dom))
        {
            for (vector<HHSequence*>::iterator itr = mSequenceList.begin(); itr != mSequenceList.end(); itr++)
            {
                string name =(*itr)->name();
                cout << "HHSequenceMgr::parseSequenceList: " << name << endl;
                map<std::string, HHSequence*>::iterator it = mSequenceMap.find(name);
                if (it == mSequenceMap.end())
                {
                    mSequenceMap[name] = *itr;
                }
                else
                {
                    cout << "More than one sequence has the same name: " << name << endl;
                }
            }
            return true;
        }
    }
    return false;
}

bool HHSequenceMgr::parseSliderList()
{
    HHXmlReader xml;
#ifdef XML_PARSE_USE_QT
    QDomDocument dom;
#else
    TiXmlDocument dom;
#endif
    if (xml.parse(FILE_SLIDERS, &dom))
    {
        if (xml.importSliders(this, mSliderList, &dom))
        {
            for (vector<HHSequence*>::iterator itr = mSliderList.begin(); itr != mSliderList.end(); itr++)
            {
                string name = (*itr)->name();
                //cout << "HHSequenceMgr::parseSliderList: " << name << endl;
                map<std::string, HHSequence*>::iterator it = mSliderMap.find(name);
                if (it == mSliderMap.end())
                {
                    mSliderMap[name] = *itr;
                }
                else
                {
                    cout << "More than one slider has the same name: " << name << endl;
                }
            }
            return true;
        }
    }
    return false;
}

HHCommandBase* HHSequenceMgr::getCommandByName(const std::string& name)
{
    std::map<std::string, HHCommandBase*>::iterator itr = mCommandMap.find(name);
    if (itr != mCommandMap.end())
    {
        return mCommandMap[name];
    }
    return NULL;
}

HHSequence* HHSequenceMgr::getSequenceByName(const std::string& name)
{
    std::map<std::string, HHSequence*>::iterator itr = mSequenceMap.find(name);
    if (itr != mSequenceMap.end())
    {
        return mSequenceMap[name];
    }
    return NULL;
}

HHSequenceSlider* HHSequenceMgr::getSliderByName(const std::string& name)
{
    std::map<std::string, HHSequence*>::iterator itr = mSliderMap.find(name);
    if (itr != mSliderMap.end())
    {
        return dynamic_cast<HHSequenceSlider*>(mSliderMap[name]);
    }
    return NULL;
}

std::vector<std::string> HHSequenceMgr::getAllSequenceNames()
{
    std::vector<std::string> names;
    for (std::vector<HHSequence*>::iterator itr = mSequenceList.begin(); itr != mSequenceList.end(); itr++)
    {
        names.push_back((*itr)->name());
    }
    return names;
}

std::vector<std::string> HHSequenceMgr::getAllSliderNames()
{
    std::vector<std::string> names;
    for (std::vector<HHSequence*>::iterator itr = mSliderList.begin(); itr != mSliderList.end(); itr++)
    {
        names.push_back((*itr)->name());
    }
    return names;
}

bool HHSequenceMgr::parseCeleX5Cfg(const string& cfgName)
{
	HHXmlReader xml;
	TiXmlDocument dom;
	if (xml.parse(cfgName, &dom))
	{
		if (xml.importCommands_CeleX5(m_CommandList_CeleX5, &dom))
		{
			for (auto itr = m_CommandList_CeleX5.begin(); itr != m_CommandList_CeleX5.end(); itr++)
			{
				//cout << "SequenceMgr::parseCeleX5Cfg: " << itr->first << endl;
				vector<HHCommandBase*> pCmdList = itr->second;
				for (auto itr1 = pCmdList.begin(); itr1 != pCmdList.end(); itr1++)
				{
					string name = (*itr1)->name();
					//cout << "----- Register Name: " << name << endl;
					map<std::string, HHCommandBase*>::iterator it = m_CommandMap_CeleX5.find(name);
					if (it == m_CommandMap_CeleX5.end())
					{
						m_CommandMap_CeleX5[name] = *itr1;
					}
					else
					{
						cout << "More than one command has the same name: " << name << endl;
					}
				}
			}
			return true;
		}
	}
	return false;
}

bool HHSequenceMgr::saveCeleX5XML(map<string, vector<CeleX5::CfgInfo>> mapCfgInfo)
{
	HHXmlReader xml;
	return xml.saveXML(mapCfgInfo);
}

map<string, vector<HHCommandBase*>> HHSequenceMgr::getCeleX5Cfg()
{
	return m_CommandList_CeleX5;
}

//***************************************************************
//***************************************************************

HHSequence::HHSequence(const std::string& name)
    : mName(name)
    , mNext("")
    , mShow(true)
    , mAdvanced(false)
{
}

HHSequence::~HHSequence()
{
}

std::string HHSequence::name()
{
    return mName;
}

bool HHSequence::isShown()
{
    return mShow;
}

void HHSequence::setShow(bool bShow)
{
    mShow = bShow;
}

bool HHSequence::isAdvanced()
{
    return mAdvanced;
}

void HHSequence::setAdvanced(bool bAdvanced)
{
    mAdvanced = bAdvanced;
}

void HHSequence::addCommand(HHCommandBase* pCmd)
{
    mCommands.push_back(pCmd);
}

bool HHSequence::fire()
{
    for (std::vector<HHCommandBase*>::iterator itr = mCommands.begin(); itr != mCommands.end(); itr++)
    {
        if (!(*itr)->valid())
        {
            return false;
        }
    }
    for (std::vector<HHCommandBase*>::iterator itr = mCommands.begin(); itr != mCommands.end(); itr++)
    {
        (*itr)->execute();
    }
    return true;
}

std::string HHSequence::getNext()
{
    return mNext;
}

void HHSequence::setNext(const std::string& nextName)
{
    mNext = nextName;
}

//***************************************************************
//***************************************************************

HHSequenceSlider::HHSequenceSlider(const std::string& name, uint32_t min, uint32_t max, uint32_t step)
    : HHSequence(name)
    , mMin(min)
    , mMax(max)
    , mStep(step)
{
}

bool HHSequenceSlider::fireWithArg(uint32_t newValue)
{
    for(std::vector<HHCommandBase*>::iterator itr = mCommands.begin(); itr != mCommands.end(); itr++)
    {
        if (!(*itr)->valid())
        {
            return false;
        }
    }
    for(std::vector<HHCommandBase*>::iterator itr = mCommands.begin(); itr != mCommands.end(); itr++)
    {
        if ((*itr)->needsArg())
            (*itr)->setValue(newValue);
        (*itr)->execute();
    }
    return true;
}

uint32_t HHSequenceSlider::getMax()
{
    return mMax;
}

uint32_t HHSequenceSlider::getMin()
{
    return mMin;
}

uint32_t HHSequenceSlider::getStep()
{
    return mStep;
}

uint32_t HHSequenceSlider::getValue()
{
    return mValue;
}

void HHSequenceSlider::setValue(uint32_t value)
{
    mValue = value;
}
