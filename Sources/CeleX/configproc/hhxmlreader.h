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

#ifndef HHXMLREADER_H
#define HHXMLREADER_H

#include <string>
#include <vector>
#include <string>
#include <stdint.h>
#include <map>

#include "../include/celex5/celex5.h"

class TiXmlDocument;
class QDomDocument;
class HHCommandBase;
class HHSequenceMgr;

class HHXmlReader
{
public:
    HHXmlReader();
    ~HHXmlReader();

    bool parse(const std::string& filename, TiXmlDocument* pDom);
    //
    bool save(const std::string& filename, TiXmlDocument* pDom);
    bool importCommands_CeleX5(std::map<std::string, std::vector<HHCommandBase*>>& commandList, TiXmlDocument* pDom);

    bool saveXML(map<string, vector<CeleX5::CfgInfo>>& mapCfgInfo);

 private:
    bool getNumber(const std::string& text, uint32_t* pNumber);
    CeleX5::CfgInfo getCfgInfoByName(string csrType, string name, map<string, vector<CeleX5::CfgInfo>>& mapCfg);
};

#endif // HHXMLREADER_H
