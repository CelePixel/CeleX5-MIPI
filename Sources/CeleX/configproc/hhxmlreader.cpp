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

#include "hhxmlreader.h"
#include "../base/xbase.h"
#include "hhwireincommand.h"
#include "hhsequencemgr.h"
#include "tinyxml/tinyxml.h"
#include <iostream>
#include <sstream>

HHXmlReader::HHXmlReader()
{
}

HHXmlReader::~HHXmlReader()
{
}

bool HHXmlReader::parse(const string &filename, TiXmlDocument *pDom)
{
    if (!pDom)
    {
        return false;
    }
    XBase base;
    string filePath = base.getApplicationDirPath();
#ifdef _WIN32
    filePath += "\\";
#endif
    filePath += filename;
	cout << filePath << endl;
    bool loadOk = pDom->LoadFile(filePath.c_str());
    if (!loadOk)
    {
        cout << "Can't load: " << pDom->ErrorDesc() << filePath;
    }
    return true;
}

bool HHXmlReader::getNumber(const string &text, uint32_t *pNumber)
{
    if (string::npos != text.find("0x") || string::npos != text.find("0X"))
    {
        *pNumber = strtoul(text.c_str(), NULL, 16);
    }
    else
    {
        *pNumber = strtoul(text.c_str(), NULL, 10);
    }
    return true;
}

bool HHXmlReader::save(const std::string& filename, TiXmlDocument* pDom)
{
	if (!pDom)
	{
		return false;
	}
	XBase base;
	string filePath = base.getApplicationDirPath();
#ifdef _WIN32
	filePath += "\\";
#endif
	filePath += filename;
	
	pDom->SaveFile(filePath.c_str());
	return true;
}

CeleX5::CfgInfo HHXmlReader::getCfgInfoByName(string csrType, string name, map<string, vector<CeleX5::CfgInfo>>& mapCfg)
{
	CeleX5::CfgInfo cfgInfo;
	for (auto itr = mapCfg.begin(); itr != mapCfg.end(); itr++)
	{
		string tapName = itr->first;
		if (csrType == tapName)
		{
			vector<CeleX5::CfgInfo> vecCfg = itr->second;
			int index = 0;
			for (auto itr1 = vecCfg.begin(); itr1 != vecCfg.end(); itr1++)
			{
				if ((*itr1).name == name)
				{
					cfgInfo = (*itr1);
					return cfgInfo;
				}
				index++;
			}
			break;
		}
	}
	return cfgInfo;
}

bool HHXmlReader::importCommands_CeleX5(map<string, vector<HHCommandBase*>> &commandList, TiXmlDocument *pDom)
{
	cout << endl << "********** HHXmlReader::importCommands_CeleX5 Begin **********" << endl;
	TiXmlElement *pRootEle = pDom->RootElement();
	if (string(pRootEle->Value()) != "commands")
	{
		cout << "Can't find commands in xml." << endl;
		return false;
	}
	for (TiXmlElement *pEle = pRootEle->FirstChildElement(); 
		NULL != pEle; 
		pEle = pEle->NextSiblingElement())
	{
		//cout << "Classification of the CSRs = " << pEle->Value() << endl;
		if (pEle->NoChildren())
		{
			//cout << "-----" << pEle->Value() << "has no children!" << endl;
			continue;
		}
		vector<HHCommandBase*> pCmdList;
		for (TiXmlElement* pChildEle = pEle->FirstChildElement(); 
			NULL != pChildEle; 
			pChildEle = pChildEle->NextSiblingElement())
		{
			std::string strCSRName = pChildEle->Value();
			//cout << "----- CSR Name = " << strCSRName << endl;
			TiXmlNode* pNode = pChildEle->FirstChild();
			WireinCommandEx* pCmd = new WireinCommandEx(strCSRName);
			while (NULL != pNode)
			{
				string tagName = pNode->Value();
				uint32_t value = 0;
				getNumber(pNode->FirstChild()->Value(), &value);
				//cout << "---------- " << tagName << " " << value << endl;
				if ("address_high" == tagName)
					pCmd->setAddress(value, WireinCommandEx::R_High);
				else if ("address_middle" == tagName)
					pCmd->setAddress(value, WireinCommandEx::R_Middle);
				else if ("address_low" == tagName)
					pCmd->setAddress(value, WireinCommandEx::R_Low);
				else if ("value_high" == tagName)
					pCmd->setValue(value, WireinCommandEx::R_High);
				else if ("value_middle" == tagName)
					pCmd->setValue(value, WireinCommandEx::R_Middle);
				else if ("value_low" == tagName)
					pCmd->setValue(value, WireinCommandEx::R_Low);
				else if ("min" == tagName)
					pCmd->setMinValue(value);
				else if ("max" == tagName)
					pCmd->setMaxValue(value);
				
				pNode = pNode->NextSiblingElement();
			}
			pCmd->setValue();
			pCmdList.push_back(pCmd);
		}
		commandList[pEle->Value()] = pCmdList;
	}
	cout << "********** HHXmlReader::importCommands_CeleX5 End **********" << endl << endl;
	return true;
}

bool HHXmlReader::saveXML(map<string, vector<CeleX5::CfgInfo>>& mapCfgInfo)
{
	TiXmlDocument*  pDom = new TiXmlDocument;
	if (parse(FILE_CELEX5_CFG, pDom))
	{
		TiXmlElement *pRootEle = pDom->RootElement();
		if (string(pRootEle->Value()) != "commands")
		{
			cout << "Can't find commands in xml." << endl;
			return false;
		}
		for (TiXmlElement *pEle = pRootEle->FirstChildElement();
			NULL != pEle;
			pEle = pEle->NextSiblingElement())
		{
			string csrType = pEle->Value();
			if ("Sensor_Core_Parameters" != csrType && "PLL_Parameters" != csrType)
			{
				continue;
			}
			cout << "Classification of the CSRs = " << csrType << endl;
			if (pEle->NoChildren())
			{
				cout << "-----" << pEle->Value() << "has no children!" << endl;
				continue;
			}
			for (TiXmlElement* pChildEle = pEle->FirstChildElement();
				NULL != pChildEle;
				pChildEle = pChildEle->NextSiblingElement())
			{
				std::string strCSRName = pChildEle->Value();
				if ("PXL_BUF_TRIM" == strCSRName)
					continue;
				cout << "----- CSR Name = " << strCSRName << endl;
				TiXmlNode* pNode = pChildEle->FirstChild();
				while (NULL != pNode)
				{
					string tagName = pNode->Value();
					string value = pNode->FirstChild()->Value();
					if ("value_high" == tagName)
					{
						CeleX5::CfgInfo cfgInfo = getCfgInfoByName(csrType, strCSRName, mapCfgInfo);
						//cout << csrType << ", " << strCSRName << endl;
						int valueH = 0;
						if (cfgInfo.low_addr == -1)
						{
							valueH = cfgInfo.value;
						}
						else
						{
							if (cfgInfo.middle_addr == -1) //no middle register
								valueH = cfgInfo.value >> 8;
							else
								valueH = cfgInfo.value >> 16;
						}
						std::stringstream ss;
						ss << valueH;
						string str = ss.str();
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
						string str = ss.str();
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
						string str = ss.str();
						//cout << str << ", " << valueL << endl;
						pNode->FirstChild()->SetValue(str.c_str());
					}
					pNode = pNode->NextSiblingElement();
				}
			}
		}
		save(FILE_CELEX5_CFG, pDom);
	}
	return true;
}

bool HHXmlReader::saveXML(const std::string& filename)
{
	TiXmlDocument*  pDom = new TiXmlDocument;
	if (parse(filename, pDom))
	{
		save(FILE_CELEX5_CFG, pDom);
	}
	return true;
}
