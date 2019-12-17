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

#include "wireincommand.h"
#include <iostream>

#define INVALID_ADDR    -1

WireinCommand::WireinCommand(const std::string& name)
	: m_strName(name)
	, m_iAddressH(-1)
	, m_iAddressM(-1)
	, m_iAddressL(-1)
	, m_iValueH(-1)
	, m_iValueM(-1)
	, m_iValueL(-1)
	, m_iValue(-1)
	, m_iMinValue(0)
	, m_iMaxValue(0)
{
}

WireinCommand::~WireinCommand() 
{ 
}

/*
*  @function :  setValue
*  @brief    :	set the value according to the register type
*  @input    :	value : the value needs to be set
*				type : register type
*  @output   :
*  @return   :  
*/
void WireinCommand::setValue(int16_t value, RegisterType type)
{
	if (type == R_High)
		m_iValueH = value;
	else if (type == R_Middle)
		m_iValueM = value;
	else if (type == R_Low)
		m_iValueL = value;
}

/*
*  @function :  combineRegBytes
*  @brief    :	set the value consisted of 24-bits data
*  @input    :	
*  @output   :
*  @return   :
*/
void WireinCommand::combineRegBytes()
{
	if (INVALID_ADDR == m_iValueL)
	{
		m_iValue = m_iValueH;
	}
	else
	{
		if (INVALID_ADDR == m_iValueM)
			m_iValue = m_iValueL + (m_iValueH << 8);
		else
			m_iValue = m_iValueL + (m_iValueM << 8) + (m_iValueH << 16);
	}
	//std::cout << "---------- value = " << m_iValue << std::endl;
}

/*
*  @function :  setAddress
*  @brief    :	set the address according to the register type
*  @input    :	address : the address needs to be set
*				type : register type
*  @output   :
*  @return   :
*/
void WireinCommand::setAddress(int16_t address, RegisterType type)
{
	if (type == R_High)
		m_iAddressH = address;
	else if (type == R_Middle)
		m_iAddressM = address;
	else if (type == R_Low)
		m_iAddressL = address;
}

/*
*  @function :  setMaxValue
*  @brief    :	set the maximum value
*  @input    :	value : the maximum value needs to be set
*  @output   :
*  @return   :
*/
void WireinCommand::setMaxValue(int32_t value)
{
	m_iMaxValue = value;
}

/*
*  @function :  setMinValue
*  @brief    :	set the minimum value
*  @input    :	value : the minimum value needs to be set
*  @output   :
*  @return   :
*/
void WireinCommand::setMinValue(int32_t value)
{
	m_iMinValue = value;
}
