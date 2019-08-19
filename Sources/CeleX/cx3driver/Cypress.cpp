#include "Cypress.h"
#include <sstream>

#define CYPRESS_DEVICE_VENDOR_ID  0x04b4//0x2560
#define CYPRESS_DEVICE_PRODUCT_ID 0x00f1//0xd051

Cypress::Cypress()
{
    //ctor
}

Cypress::~Cypress()
{
    //dtor
}

bool Cypress::open_usb(void)
{
	if (usb_open(CYPRESS_DEVICE_VENDOR_ID, CYPRESS_DEVICE_PRODUCT_ID, LIBUSB_TRANSFER_TYPE_BULK) == true)
	{
		return true;
	}
	usb_close();
	return false;
}

bool Cypress::open_stream(void)
{
	//开启视频流
	uint8_t video_start[] = { 0x00, 0x00,0x01,0x02,0x0A,0x8B,0x02,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x20,0x1C,0x00, 0x00,0x90,0x00, 0x00 };
	//usb_control(0x41, 0x99, 0x00, 0x00, nullptr, 0);
	if (usb_control(0x21, 0x01, 0x201, 0x0, video_start, sizeof(video_start)) == true)
	{
		if (start() == true)
		{
			return true;
		}
		usb_control(0x41, 0x88, 0x00, 0x00, nullptr, 0);
	}
	return false;
}

void Cypress::close_usb(void)
{
	usb_close();
}

void Cypress::close_stream(void)
{
	usb_control(0x41, 0x88, 0x00, 0x00, nullptr, 0);
	stop();
}

bool Cypress::USB_Set(uint16_t wId, uint16_t Reg, uint16_t Value)
{
    unsigned char data[10];

    *(uint16_t*)data = Reg;
    *(uint16_t*)(data+2) = Value;
    return usb_control(0x21, 0x1,wId, 0x300,data,4);
}

bool Cypress::USB_Get(uint16_t wId, uint16_t Reg, uint16_t &Value)
{
    unsigned char data[10];

    *(uint16_t*)data = Reg;
    if (usb_control(0x21,0x1, wId, 0x300,data,4) == true)
    {
        if (usb_control(0xA1,0x81, wId, 0x300,data,4) == true)
        {
            Value = *(uint16_t*)(data+2);
            return true;
        }
    }
    return false;
}

bool Cypress::writeSerialNumber(std::string number)
{
	uint8_t data[32] = { "\0"  };
	uint16_t address = 0x0001;
	*(uint16_t*)data = address;
	for (int i = 0; i < number.size(); i++)
		data[i+2] = number.at(i);
	return usb_control(0x21, 0x1, 0x500, 0x300, data, number.size()+2);
	return true;
}

std::string Cypress::getSerialNumber()
{
	char data_read[32] = { "\0" };
	uint16_t address = 0x0001;
	*(uint16_t*)data_read = address;
	if (usb_control(0x21, 0x01, 0x600, 0x300, (uint8_t*)data_read, 32))
	{
		if (usb_control(0xA1, 0x81, 0x600, 0x300, (uint8_t*)data_read, 32))
		{
			return(std::string(&data_read[2]));
		}
	}
	return std::string("");
}

std::string Cypress::getFirmwareVersion()
{
	char data_read[5] = { "\0" };
	uint16_t address = 0x0002;
	*(uint16_t*)data_read = address;
	if (usb_control(0x21, 0x01, 0x600, 0x300, (uint8_t*)data_read, 4))
	{
		if (usb_control(0xA1, 0x81, 0x600, 0x300, (uint8_t*)data_read, 4))
		{
			std::string str;
			std::stringstream ss;
			ss << (int)data_read[3];
			ss << ".";
			ss << (int)data_read[2];
			str = std::string(ss.str());

			return str;
		}
	}
	return std::string("");
}

std::string Cypress::getFirmwareDate()
{
	char data_read[32] = { "\0" };
	uint16_t address = 0x0003;
	*(uint16_t*)data_read = address;
	if (usb_control(0x21, 0x01, 0x600, 0x300, (uint8_t*)data_read, 32))
	{
		if (usb_control(0xA1, 0x81, 0x600, 0x300, (uint8_t*)data_read, 32))
		{
			return(std::string(&data_read[2]));
		}
	}
	return std::string("");
}