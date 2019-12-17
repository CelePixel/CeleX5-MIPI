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

#ifndef CELEX5CFGMGR_H
#define CELEX5CFGMGR_H

#include <string>
#include <vector>
#include <map>
#include <stdint.h>
#include "../include/celex5/celex5.h"

class WireinCommand;

/*
*  This class is used to process the config file of CeleX5.
*  Including parse the config file, save the file and get
*  the parameters in the config file.
*/
class CeleX5CfgMgr
{
public:
	CeleX5CfgMgr();
	~CeleX5CfgMgr();
	
	bool parseCeleX5Cfg(const std::string& cfgName);
	bool saveCeleX5XML(std::map<std::string, std::vector<CeleX5::CfgInfo>>& mapCfgInfo);
	bool saveCeleX5XML(const std::string& cfgName);
	std::map<std::string, std::vector<WireinCommand*>> getCeleX5Cfg();

private:
	std::map<std::string, WireinCommand*>                 m_CommandMapCeleX5;
	std::map<std::string, std::vector<WireinCommand*>>    m_CommandListCeleX5;
};

#endif // CELEX5CFGMGR_H
