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

#ifndef HHWIREINCOMMAND_H
#define HHWIREINCOMMAND_H

#include <stdint.h>
#include "hhcommand.h"
//#include <map>
#include <iostream>

class FrontPanel;

class HHWireinCommand : public HHCommandBase
{
public:
    HHWireinCommand(const std::string& name);

    virtual void execute() override;

    virtual void setValue(uint32_t value) override;
    void setAddress(uint32_t address);
    void setMask(uint32_t mask);

    virtual HHCommandBase* clone() override;

private:
    uint32_t     m_uiAddress;
    uint32_t     m_uiValue;
    uint32_t     m_uiMask;
};

class WireinCommandEx : public HHCommandBase
{
public:
	enum RegisterType
	{
		R_High = 0,
		R_Middle = 1,
		R_Low = 2
	};
	WireinCommandEx(const std::string& name)
		: HHCommandBase(name)
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
	~WireinCommandEx() { }

	virtual void execute() {}
	virtual void setValue(uint32_t value) {}
	virtual HHCommandBase* clone() { return NULL; }

	void setValue(int16_t value, RegisterType type)
	{
		if (type == R_High)
			m_iValueH = value;
		else if (type == R_Middle)
			m_iValueM = value;
		else if (type == R_Low)
			m_iValueL = value;
	}
	void setAddress(int16_t address, RegisterType type)
	{
		if (type == R_High)
			m_iAddressH = address;
		else if (type == R_Middle)
			m_iAddressM = address;
		else if (type == R_Low)
			m_iAddressL = address;
	}

	void setValue()
	{
		if (-1 == m_iValueL)
		{
			m_iValue = m_iValueH;
		}
		else
		{
			if (-1 == m_iValueM)
				m_iValue = m_iValueL + (m_iValueH << 8);
			else
				m_iValue = m_iValueL + (m_iValueM << 8)  + (m_iValueH << 16);
		}
		//std::cout << "---------- value = " << m_iValue << std::endl;
	}

	void setMaxValue(int32_t value)
	{
		m_iMaxValue = value;
	}
	void setMinValue(int32_t value)
	{
		m_iMinValue = value;
	}

	int32_t value() { return m_iValue; }
	int16_t highAddr() { return m_iAddressH; }
	int16_t middleAddr() { return m_iAddressM; }
	int16_t lowAddr() { return m_iAddressL; }
	int32_t maxValue() { return m_iMaxValue; }
	int32_t minValue() { return m_iMinValue; }

private:
	//std::map<int16_t, int16_t>  m_mapRegister; //key: address; value: value
	int16_t     m_iAddressH;
	int16_t     m_iAddressM;
	int16_t     m_iAddressL;
	int16_t     m_iValueH;
	int16_t     m_iValueM;
	int16_t     m_iValueL;
	int32_t     m_iValue;
	int32_t     m_iMinValue;
	int32_t     m_iMaxValue;
};

#endif // HHWIREINCOMMAND_H
