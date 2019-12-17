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

#include <iostream>
#include <iostream>
#include "celex5cfgmgr.h"
#include "tinyxml/tinyxml.h"
#include "../include/celextypes.h"
#include "wireincommand.h"
#include "xmlreader.h"

CeleX5CfgMgr::CeleX5CfgMgr()
{
}

CeleX5CfgMgr::~CeleX5CfgMgr()
{
}

/*
*  @function :  parseCeleX5Cfg
*  @brief    :	parse the config file of CeleX5 
*  @input    :  cfgName : name of the config file that needs to be parsed
*  @output   :	
*  @return   :	bool : true for parsing sucessfully; false for failed
*/
bool CeleX5CfgMgr::parseCeleX5Cfg(const std::string& cfgName)
{
	XmlReader xml;
	TiXmlDocument dom;
	std::vector<WireinCommand*> pCmdList;
	std::string name = "";
	if (xml.parse(cfgName, &dom))
	{
		if (xml.importCeleX5Commands(m_CommandListCeleX5, &dom))
		{
			for (auto itr = m_CommandListCeleX5.begin(); itr != m_CommandListCeleX5.end(); ++itr)
			{
				//cout << "CeleX5CfgMgr::parseCeleX5Cfg: " << itr->first << endl;
				pCmdList = itr->second;
				for (auto cmd = pCmdList.begin(); cmd != pCmdList.end(); ++cmd)
				{
					name = (*cmd)->getName();
					//cout << "----- Register Name: " << name << endl;
					if (m_CommandMapCeleX5.find(name) == m_CommandMapCeleX5.end())
					{
						m_CommandMapCeleX5[name] = *cmd;
					}
					else
					{
						std::cout << "More than one command has the same name: " << name << std::endl;
					}
				}
			}
			return true;
		}
	}
	return false;
}

/*
*  @function :  saveCeleX5XML
*  @brief    :	saves the config file after modification
*  @input    :  mapCfgInfo : parameters in the config file
*  @output   :	
*  @return   :	bool : true for saving sucessfully; false for failed
*/
bool CeleX5CfgMgr::saveCeleX5XML(std::map<std::string, std::vector<CeleX5::CfgInfo>>& mapCfgInfo)
{
	XmlReader xml;
	return xml.saveXML(mapCfgInfo);
}

/*
*  @function :  saveCeleX5XML
*  @brief    :	save the config file if the xml file not exists when sensor first launchs
*  @input    :  cfgName : name of the config file that needs to be saved
*  @output   :	
*  @return   :  bool : true for saving sucessfully; false for failed
*/
bool CeleX5CfgMgr::saveCeleX5XML(const std::string& cfgName)
{
	XmlReader xml;
	return xml.saveXML(cfgName);
}

/*
*  @function :  getCeleX5Cfg
*  @brief    :	get the parameters in the config file 
*  @input    :  
*  @output   :	
*  @return   :  key-value type parameters in the config file
*/
std::map<std::string, std::vector<WireinCommand*>> CeleX5CfgMgr::getCeleX5Cfg()
{
	return m_CommandListCeleX5;
}
