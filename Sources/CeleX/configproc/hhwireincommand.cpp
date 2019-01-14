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

#include "hhwireincommand.h"
#include "../frontpanel/frontpanel.h"
#include <iostream>

HHWireinCommand::HHWireinCommand(const std::string& name)
    :HHCommandBase(name)
{
    m_uiMask = 0;
    m_uiValue = 0;
    m_uiAddress = 0;
}

void HHWireinCommand::execute()
{
    FrontPanel::getInstance()->wireIn(m_uiAddress, m_uiValue, m_uiMask);
    cout << "Address: " << m_uiAddress << "; Value: " << m_uiValue << "; Mask: " << m_uiMask << endl;
}

void HHWireinCommand::setAddress(uint32_t address)
{
    m_uiAddress = address;
}

void HHWireinCommand::setMask(uint32_t mask)
{
    m_uiMask = mask;
}

void HHWireinCommand::setValue(uint32_t value)
{
    int tmpmask = m_uiMask;
    int shift = 0;
    while ((tmpmask % 2 == 0) && (tmpmask != 0))
    {
        tmpmask = tmpmask / 2;
        shift ++;
    }
    m_uiValue = value << shift;
}

HHCommandBase *HHWireinCommand::clone()
{
    HHWireinCommand* pCmd = new HHWireinCommand(this->m_strName);
    pCmd->valid(this->valid());
    pCmd->needsArg(this ->needsArg());
    pCmd->error(this->error());
    pCmd->m_uiAddress = this->m_uiAddress;
    pCmd->m_uiValue = this->m_uiValue;
    pCmd->m_uiMask = this->m_uiMask;

    return pCmd;
}
