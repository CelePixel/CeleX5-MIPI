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

#include <sstream>
#include "cypress.h"

#define CYPRESS_DEVICE_VENDOR_ID	0x04b4 //0x2560
#define CYPRESS_DEVICE_PRODUCT_ID	0x00f1 //0xd051

#define ADDRESS_SERIAL_NUMBER       0x0001
#define ADDRESS_FIRMWARE_VERSION    0x0002
#define ADDRESS_FIRMWARE_DATE       0x0003

Cypress::Cypress()
{

}

Cypress::~Cypress()
{

}

/*
*  @function: openUSB
*  @brief   : open usb
*  @input   :
*  @output  :
*  @return  : true is successful to open usb
*/
bool Cypress::openUSB(void)
{
	if (usbOpen(CYPRESS_DEVICE_VENDOR_ID, CYPRESS_DEVICE_PRODUCT_ID, LIBUSB_TRANSFER_TYPE_BULK))
	{
		return true;
	}
	usbClose();
	return false;
}

/*
*  @function: openStream
*  @brief   : open usb stream
*  @input   :
*  @output  :
*  @return  : true is successful to open usb stream
*/
bool Cypress::openStream(void)
{
	//开启视频流
	uint8_t video_start[] = { 0x00, 0x00,0x01,0x02,0x0A,0x8B,0x02,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x20,0x1C,0x00, 0x00,0x90,0x00, 0x00 };
	//usb_control(0x41, 0x99, 0x00, 0x00, nullptr, 0);
	if (usbControl(0x21, 0x01, 0x201, 0x0, video_start, sizeof(video_start)))
	{
		if (start() == true)
		{
			return true;
		}
		usbControl(0x41, 0x88, 0x00, 0x00, nullptr, 0);
	}
	return false;
}

/*
*  @function: closeUSB
*  @brief   : close usb
*  @input   :
*  @output  :
*  @return  : 
*/
void Cypress::closeUSB(void)
{
	usbClose();
}

/*
*  @function: closeStream
*  @brief   : close usb stream
*  @input   :
*  @output  :
*  @return  :
*/
void Cypress::closeStream(void)
{
	usbControl(0x41, 0x88, 0x00, 0x00, nullptr, 0);
	stop();
}

/*
*  @function: usbSet
*  @brief   : 
*  @input   : wId: control type
*             reg: register address
*             value: register value
*  @output  :
*  @return  : true is successful to perform a USB control transfer
*/
bool Cypress::usbSet(uint16_t wId, uint16_t reg, uint16_t value)
{
    unsigned char data[10];

    *(uint16_t*)data = reg;
    *(uint16_t*)(data+2) = value;
    return usbControl(0x21, 0x1, wId, 0x300, data, 4);
}

/*
*  @function: usbGet
*  @brief   : 
*  @input   : wId: control type
*             reg: register address   
*  @output  : value: register value
*  @return  : true is successful to get data from USB control transfer
*/
bool Cypress::usbGet(uint16_t wId, uint16_t reg, uint16_t &value)
{
    unsigned char data[10];

    *(uint16_t*)data = reg;
    if (usbControl(0x21, 0x1, wId, 0x300, data, 4))
    {
        if (usbControl(0xA1, 0x81, wId, 0x300, data, 4))
        {
            value = *(uint16_t*)(data+2);
            return true;
        }
    }
    return false;
}

/*
*  @function: writeSerialNumber
*  @brief   : write the serial number of sensor
*  @input   : number: sensor serial number
*  @output  :
*  @return  : true is successful to write sensor serial number
*/
bool Cypress::writeSerialNumber(std::string number)
{
	uint8_t data[32] = { "\0" };
	uint16_t address = ADDRESS_SERIAL_NUMBER;
	*(uint16_t*)data = address;
	for (int i = 0; i < number.size(); i++)
		data[i+2] = number.at(i);
	return usbControl(0x21, 0x1, 0x500, 0x300, data, number.size()+2);
}

/*
*  @function: getSerialNumber
*  @brief   : get the serial number of sensor
*  @input   :
*  @output  :
*  @return  : sensor serial number
*/
std::string Cypress::getSerialNumber()
{
	char data_read[32] = { "\0" };
	uint16_t address = ADDRESS_SERIAL_NUMBER;
	*(uint16_t*)data_read = address;
	if (usbControl(0x21, 0x01, 0x600, 0x300, (uint8_t*)data_read, 32))
	{
		if (usbControl(0xA1, 0x81, 0x600, 0x300, (uint8_t*)data_read, 32))
		{
			return(std::string(&data_read[2]));
		}
	}
	return std::string("");
}

/*
*  @function: getFirmwareVersion
*  @brief   : get firmware version
*  @input   :
*  @output  :
*  @return  : firmware version
*/
std::string Cypress::getFirmwareVersion()
{
	char data_read[5] = { "\0" };
	uint16_t address = ADDRESS_FIRMWARE_VERSION;
	*(uint16_t*)data_read = address;
	if (usbControl(0x21, 0x01, 0x600, 0x300, (uint8_t*)data_read, 4))
	{
		if (usbControl(0xA1, 0x81, 0x600, 0x300, (uint8_t*)data_read, 4))
		{
			std::string str;
			std::stringstream ss;
			ss << static_cast<int>(data_read[3]);
			ss << ".";
			ss << static_cast<int>(data_read[2]);
			str = std::string(ss.str());

			return str;
		}
	}
	return std::string("");
}

/*
*  @function: getFirmwareDate
*  @brief   : get firmware date
*  @input   :
*  @output  :
*  @return  : firmware date
*/
std::string Cypress::getFirmwareDate()
{
	char dataRead[32] = { "\0" };
	uint16_t address = ADDRESS_FIRMWARE_DATE;
	*(uint16_t*)dataRead = address;
	if (usbControl(0x21, 0x01, 0x600, 0x300, (uint8_t*)dataRead, 32))
	{
		if (usbControl(0xA1, 0x81, 0x600, 0x300, (uint8_t*)dataRead, 32))
		{
			return(std::string(&dataRead[2]));
		}
	}
	return std::string("");
}
