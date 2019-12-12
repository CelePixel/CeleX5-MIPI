/*
* Copyright (c) 2017-2020  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef CYPRESS_H
#define CYPRESS_H

#include "usbinterface.h"

typedef enum
{
    CTRL_I2C_SET_REG  = 0x100,
    CTRL_I2C_GET_REG  = 0x200,
    CTRL_MIPI_SET_REG = 0x300,
    CTRL_MIPI_GET_REG = 0x400,
}CONTROL_COMMAND;

class Cypress : public USBInterface
{
    public:
        Cypress();
        virtual ~Cypress();

		bool openUSB(void); 
		bool openStream(void); 

		void closeUSB(void); 
		void closeStream(void);

        bool usbSet(uint16_t wId, uint16_t reg, uint16_t value);
        bool usbGet(uint16_t wId, uint16_t reg, uint16_t &value);

		bool writeSerialNumber(std::string number);
		std::string getSerialNumber();
		std::string getFirmwareVersion();
		std::string getFirmwareDate();
};

#endif // CYPRESS_H
