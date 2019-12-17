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

#ifndef WIREINCOMMAND_H
#define WIREINCOMMAND_H

#include <stdint.h>
#include <iostream>

/*
*  This class is used to set and get the parameters of register .
*/
class WireinCommand
{
public:
	enum RegisterType
	{
		R_High = 0,
		R_Middle = 1,
		R_Low = 2
	};
	WireinCommand(const std::string& name);

	~WireinCommand();

	void setValue(int16_t value, RegisterType type);
	void combineRegBytes();

	void setAddress(int16_t address, RegisterType type);

	void setMaxValue(int32_t value);
	void setMinValue(int32_t value);

	std::string getName() { return m_strName; }
	int32_t getValue() { return m_iValue; }
	int16_t getHighAddr() { return m_iAddressH; }
	int16_t getMiddleAddr() { return m_iAddressM; }
	int16_t getLowAddr() { return m_iAddressL; }
	int32_t getMaxValue() { return m_iMaxValue; }
	int32_t getMinValue() { return m_iMinValue; }

private:
	//std::map<int16_t, int16_t>  m_mapRegister; //key: address; value: value
	std::string     m_strName;
	int16_t         m_iAddressH;
	int16_t         m_iAddressM;
	int16_t         m_iAddressL;
	int16_t         m_iValueH;
	int16_t         m_iValueM;
	int16_t         m_iValueL;
	int32_t         m_iValue;
	int32_t         m_iMinValue;
	int32_t         m_iMaxValue;
};

#endif // WIREINCOMMAND_H
