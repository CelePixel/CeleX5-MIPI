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

#ifndef HHCOMMAND_H
#define HHCOMMAND_H

#include <string>
#include <stdint.h>

class FrontPanel;

class HHCommandBase
{
public:
    HHCommandBase(const std::string& name);
    ~HHCommandBase();

    std::string name();

    virtual void execute() = 0;
    virtual bool valid();
    virtual void valid(bool bValid);
    virtual std::string error();
    virtual void error(const std::string& error);

    virtual void needsArg(bool bNeed);
    virtual bool needsArg();

    virtual void setValue(uint32_t value) = 0;

    virtual HHCommandBase* clone();

protected:
    std::string     m_strName;

private:
    bool            m_bValid;
    bool            m_bNeedsArg;
    std::string     m_strErrorMessage;
};

#endif // HHCOMMAND_H
