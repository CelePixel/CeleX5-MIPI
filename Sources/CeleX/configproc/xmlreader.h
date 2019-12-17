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

#ifndef XMLREADER_H
#define XMLREADER_H

#include <string>
#include <vector>
#include <string>
#include <stdint.h>
#include <map>

#include "../include/celex5/celex5.h"

class TiXmlDocument;
class WireinCommand;

/*
*  This class is used to process the xml file.
*  Including parse the xml file, save the file.
*/
class XmlReader
{
public:
    XmlReader();
	~XmlReader();
	
	bool parse(const std::string& filename, TiXmlDocument* pDom);
	bool save(const std::string& filename, TiXmlDocument* pDom);
	bool importCeleX5Commands(std::map<std::string, std::vector<WireinCommand*> >& commandList, TiXmlDocument* pDom);
	
	bool saveXML(const std::map<std::string, std::vector<CeleX5::CfgInfo> >& mapCfgInfo);
	bool saveXML(const std::string& filename);

 private:
    bool getNumber(const std::string& text, uint32_t* pNumber);
    CeleX5::CfgInfo getCfgInfoByName(const std::string& csrType, const std::string& name, const std::map<std::string, std::vector<CeleX5::CfgInfo> >& mapCfg);
};

#endif // XMLREADER_H
