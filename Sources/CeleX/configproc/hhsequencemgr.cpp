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
#include "tinyxml/tinyxml.h"
#include "../include/celextypes.h"

using namespace std;

HHSequenceMgr::HHSequenceMgr()
{
}

HHSequenceMgr::~HHSequenceMgr()
{
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

<<<<<<< HEAD
=======
bool HHSequenceMgr::saveCeleX5XML(const string& cfgName)
{
	HHXmlReader xml;
	return xml.saveXML(cfgName);
}

>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
map<string, vector<HHCommandBase*>> HHSequenceMgr::getCeleX5Cfg()
{
	return m_CommandList_CeleX5;
}