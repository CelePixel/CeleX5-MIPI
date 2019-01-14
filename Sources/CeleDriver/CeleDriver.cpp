#include <string.h>
#include <stdio.h>
#include "CeleDriver.h"
#include "Cypress.h"

Cypress Cyp;

CeleDriver::CeleDriver(void)
{

}

CeleDriver::~CeleDriver(void)
{

}

bool CeleDriver::Open(void)
{
    return Cyp.StreamOn();
}

bool CeleDriver::openUSB()
{
	return Cyp.open_usb();
}

bool CeleDriver::openStream()
{
	return Cyp.open_stream();
}

void CeleDriver::Close(void)
{
	Cyp.StreamOff();
}

void CeleDriver::closeUSB()
{
	Cyp.close_usb();
}

void CeleDriver::closeStream()
{
	Cyp.close_stream();
}

bool CeleDriver::i2c_set(uint16_t reg,uint16_t value)
{
    return Cyp.USB_Set(CTRL_I2C_SET_REG,reg,value);
}

bool CeleDriver::i2c_get(uint16_t reg,uint16_t &value)
{
    return Cyp.USB_Get(CTRL_I2C_GET_REG,reg,value);
}

bool CeleDriver::mipi_set(uint16_t reg,uint16_t value)
{
    return Cyp.USB_Set(CTRL_MIPI_SET_REG,reg,value);
}

bool CeleDriver::mipi_get(uint16_t reg,uint16_t &value)
{
    return Cyp.USB_Get(CTRL_MIPI_GET_REG,reg,value);
}

bool CeleDriver::getimage(vector<uint8_t> &image)
{
    return GetPicture(image);
}

bool CeleDriver::getPackage(unsigned char* buffer, uint32_t* length)
{
	return GetPackage(buffer, length);
}

void CeleDriver::clearData()
{
	ClearData();
}