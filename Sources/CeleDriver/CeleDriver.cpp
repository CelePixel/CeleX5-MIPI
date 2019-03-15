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

bool CeleDriver::writeSerialNumber(std::string number)
{
	return Cyp.writeSerialNumber(number);
}

std::string CeleDriver::getSerialNumber()
{
	return Cyp.getSerialNumber();
}

std::string CeleDriver::getFirmwareVersion()
{
	return Cyp.getFirmwareVersion();
}

std::string CeleDriver::getFirmwareDate()
{
	return Cyp.getFirmwareDate();
}

bool CeleDriver::getIMUData(uint8_t* buffer, long& time_stamp, long& time_stamp2)
{
	return Cyp.get_imu_data(buffer, time_stamp, time_stamp2);
}

bool CeleDriver::getimage(std::vector<uint8_t> &image)
{
    return GetPicture(image);
}

bool CeleDriver::getimage(std::vector<uint8_t> &image, long& time_stamp1, long& time_stamp2)
{
	return GetPicture(image, time_stamp1, time_stamp2);
}

bool CeleDriver::getSensorData(std::vector<uint8_t> &image, std::time_t& time_stamp_end, std::vector<IMU_Raw_Data>& imu_data)
{
	return GetPicture(image, time_stamp_end, imu_data);
}

void CeleDriver::clearData()
{
	ClearData();
}

void CeleDriver::SetCallback(libcelex_transfer_cb_fn callback)
{
	printf("------------------ CeleDriver::SetCallback\n");
	setCallback(callback);
}
