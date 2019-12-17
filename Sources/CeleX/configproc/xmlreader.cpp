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
#include <sstream>
#include "xmlreader.h"
#include "wireincommand.h"
#include "celex5cfgmgr.h"
#include "tinyxml/tinyxml.h"
#include "../base/filedirectory.h"

#define INVALID_ADDR -1

XmlReader::XmlReader()
{
}

XmlReader::~XmlReader()
{
}

/*
*  @function :  parse
*  @brief    :	parse the config file and load it
*  @input    :  filename : name of the config file that needs to be parsed
*  @output   :	pDom : a document binds together all the XML pieces
*  @return   :	bool : true for loading sucessfully; false for failed
*/
bool XmlReader::parse(const std::string &filename, TiXmlDocument *pDom)
{
    if (pDom == nullptr)
    {
        return false;
    }
    FileDirectory dir;
    std::string filePath = dir.getApplicationDirPath();
#ifdef _WIN32
    filePath += "\\";
#endif
    filePath += filename;
    bool loadOk = pDom->LoadFile(filePath.c_str());
    if (!loadOk)
    {
        std::cout << "Can't load: " << pDom->ErrorDesc() << filePath;
    }
    return true;
}

/*
*  @function :  save
*  @brief    :	save the config file
*  @input    :  filename : name of the config file that needs to be saved
*  @output   :	pDom : a document binds together all the XML pieces
*  @return   :	bool : true for saving sucessfully; false for failed
*/
bool XmlReader::save(const std::string& filename, TiXmlDocument* pDom)
{
	if (pDom == nullptr)
	{
		return false;
	}
	FileDirectory dir;
	std::string filePath = dir.getApplicationDirPath();
#ifdef _WIN32
	filePath += "\\";
#endif
	filePath += filename;
	
	pDom->SaveFile(filePath.c_str());
	return true;
}

/*
*  @function :  importCommandsOfCeleX5
*  @brief    :	import the command parameters in the config file for CeleX5
*  @input    :  commandList : command parameters in the config file that needs to be imported
*				pDom : a document binds together all the XML pieces
*  @output   :	
*  @return   :	bool : true for importing sucessfully; false for failed
*/
bool XmlReader::importCeleX5Commands(std::map<std::string, std::vector<WireinCommand*>> &commandList, TiXmlDocument *pDom)
{
	std::cout << std::endl << "********** HHXmlReader::importCommands_CeleX5 Begin **********" << std::endl;
	TiXmlElement *pRootEle = pDom->RootElement();
	if (std::string(pRootEle->Value()) != "commands")
	{
		std::cout << "Can't find commands in xml." << std::endl;
		return false;
	}
	for (TiXmlElement *pEle = pRootEle->FirstChildElement(); 
		nullptr != pEle; 
		pEle = pEle->NextSiblingElement())
	{
		//cout << "Classification of the CSRs = " << pEle->Value() << endl;
		if (pEle->NoChildren())
		{
			//cout << "-----" << pEle->Value() << "has no children!" << endl;
			continue;
		}
		std::vector<WireinCommand*> pCmdList;
		std::string strCSRName;
		TiXmlNode* pNode;
		WireinCommand* pCmd;
		for (TiXmlElement* pChildEle = pEle->FirstChildElement(); 
			nullptr != pChildEle;
			pChildEle = pChildEle->NextSiblingElement())
		{
			strCSRName = pChildEle->Value();
			//cout << "----- CSR Name = " << strCSRName << endl;
			pNode = pChildEle->FirstChild();
			pCmd = new WireinCommand(strCSRName);
			if (pCmd == nullptr)
				return false;
			while (nullptr != pNode)
			{
				std::string tagName = pNode->Value();
				uint32_t value = 0;
				getNumber(pNode->FirstChild()->Value(), &value);
				//cout << "---------- " << tagName << " " << value << endl;
				if ("address_high" == tagName)
					pCmd->setAddress(value, WireinCommand::R_High);
				else if ("address_middle" == tagName)
					pCmd->setAddress(value, WireinCommand::R_Middle);
				else if ("address_low" == tagName)
					pCmd->setAddress(value, WireinCommand::R_Low);
				else if ("value_high" == tagName)
					pCmd->setValue(value, WireinCommand::R_High);
				else if ("value_middle" == tagName)
					pCmd->setValue(value, WireinCommand::R_Middle);
				else if ("value_low" == tagName)
					pCmd->setValue(value, WireinCommand::R_Low);
				else if ("min" == tagName)
					pCmd->setMinValue(value);
				else if ("max" == tagName)
					pCmd->setMaxValue(value);
				
				pNode = pNode->NextSiblingElement();
			}
			pCmd->combineRegBytes();
			pCmdList.push_back(pCmd);
		}
		commandList[pEle->Value()] = pCmdList;
	}
	std::cout << "********** HHXmlReader::importCommands_CeleX5 End **********" << std::endl << std::endl;
	return true;
}

/*
*  @function :  saveXML
*  @brief    :	save the config xml file after parameters modified
*  @input    :  mapCfgInfo : parameters in the config file
*  @output   :
*  @return   :	bool : true for saving sucessfully; false for failed
*/
bool XmlReader::saveXML(const std::map<std::string, std::vector<CeleX5::CfgInfo>>& mapCfgInfo)
{
	TiXmlDocument*  pDom = new TiXmlDocument;
	if (parse(FILE_CELEX5_CFG, pDom))
	{
		TiXmlElement *pRootEle = pDom->RootElement();
		std::string csrType;
		std::string strCSRName;
		TiXmlNode* pNode;
		if (std::string(pRootEle->Value()) != "commands")
		{
			std::cout << "Can't find commands in xml." << std::endl;
			return false;
		}
		for (TiXmlElement *pEle = pRootEle->FirstChildElement();
			nullptr != pEle;
			pEle = pEle->NextSiblingElement())
		{
			csrType = pEle->Value();
			if ("Sensor_Core_Parameters" != csrType && "PLL_Parameters" != csrType)
			{
				continue;
			}
			//cout << "Classification of the CSRs = " << csrType << endl;
			if (pEle->NoChildren())
			{
				std::cout << "-----" << pEle->Value() << "has no children!" << std::endl;
				continue;
			}
			for (TiXmlElement* pChildEle = pEle->FirstChildElement();
				nullptr != pChildEle;
				pChildEle = pChildEle->NextSiblingElement())
			{
				strCSRName = pChildEle->Value();
				if ("PXL_BUF_TRIM" == strCSRName)
					continue;
				//cout << "----- CSR Name = " << strCSRName << endl;
				pNode = pChildEle->FirstChild();
				while (nullptr != pNode)
				{
					std::string tagName = pNode->Value();
					std::string value = pNode->FirstChild()->Value();
					if ("value_high" == tagName)
					{
						CeleX5::CfgInfo cfgInfo = getCfgInfoByName(csrType, strCSRName, mapCfgInfo);
						//cout << csrType << ", " << strCSRName << endl;
						int valueH = 0;
						if (cfgInfo.lowAddr == INVALID_ADDR)
						{
							valueH = cfgInfo.value;
						}
						else
						{
							if (cfgInfo.middleAddr == INVALID_ADDR) //no middle register
								valueH = cfgInfo.value >> 8;
							else
								valueH = cfgInfo.value >> 16;
						}
						std::stringstream ss;
						ss << valueH;
						std::string str = ss.str();
						//cout << str << ", " << valueH << endl;
						pNode->FirstChild()->SetValue(str.c_str());
					}
					else if ("value_middle" == tagName)
					{
						CeleX5::CfgInfo cfgInfo = getCfgInfoByName(csrType, strCSRName, mapCfgInfo);
						//cout << csrType << ", " << strCSRName << endl;
						uint32_t valueM = (0xFF00 & cfgInfo.value) >> 8;
						std::stringstream ss;
						ss << valueM;
						std::string str = ss.str();
						//cout << str << ", " << valueM << endl;
						pNode->FirstChild()->SetValue(str.c_str());
					}
					else if ("value_low" == tagName)
					{
						CeleX5::CfgInfo cfgInfo = getCfgInfoByName(csrType, strCSRName, mapCfgInfo);
						//cout << csrType << ", " << strCSRName << endl;
						uint32_t valueL = 0xFF & cfgInfo.value;
						std::stringstream ss;
						ss << valueL;
						std::string str = ss.str();
						//cout << str << ", " << valueL << endl;
						pNode->FirstChild()->SetValue(str.c_str());
					}
					pNode = pNode->NextSiblingElement();
				}
			}
		}
		save(FILE_CELEX5_CFG, pDom);
		return true;
	}
	else
	{
		return false;
	}
}

/*
*  @function :  saveXML
*  @brief    :	save the config file to the .xml file
*  @input    :  filename : name of the config file that needs to be saved
*  @output   :
*  @return   :	bool : true for saving sucessfully; false for failed
*/
bool XmlReader::saveXML(const std::string& filename)
{
	TiXmlDocument*  pDom = new TiXmlDocument;
	if (parse(filename, pDom))
	{
		save(FILE_CELEX5_CFG, pDom);
	}
	return true;
}

/*
*  @function :  
*  @brief    :	convert the string to uint32
*  @input    :  text : string that needs to be converted
*				pNumber : the number converted from string
*  @output   :	
*  @return   :	bool : true for converting sucessfully; false for failed
*/
bool XmlReader::getNumber(const std::string &text, uint32_t *pNumber)
{
	if (std::string::npos != text.find("0x") || std::string::npos != text.find("0X"))
	{
		*pNumber = strtoul(text.c_str(), nullptr, 16);
	}
	else
	{
		*pNumber = strtoul(text.c_str(), nullptr, 10);
	}
	return true;
}

/*
*  @function :  getCfgInfoByName
*  @brief    :	get the config parameters by each register
*  @input    :  csrType : the group type of the registers
*				name : the name of register 
*				mapCfg :  parameters in the config file
*  @output   :	
*  @return   :	configuration infomations of each register
*/
CeleX5::CfgInfo XmlReader::getCfgInfoByName(const std::string& csrType, const std::string& name, const std::map<std::string, std::vector<CeleX5::CfgInfo>>& mapCfg)
{
	CeleX5::CfgInfo cfgInfo;
	for (auto itr = mapCfg.begin(); itr != mapCfg.end(); itr++)
	{
		std::string tapName = itr->first;
		if (csrType == tapName)
		{
			std::vector<CeleX5::CfgInfo> vecCfg = itr->second;
			for (auto itrCfg = vecCfg.begin(); itrCfg != vecCfg.end(); itrCfg++)
			{
				if ((*itrCfg).name == name)
				{
					cfgInfo = (*itrCfg);
					return cfgInfo;
				}
			}
			break;
		}
	}
	return cfgInfo;
}
