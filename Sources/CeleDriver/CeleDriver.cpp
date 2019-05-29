#include <string.h>
#include <stdio.h>
#include "CeleDriver.h"
#include "Cypress.h"

CeleDriver::CeleDriver(void)
{
	m_pCypress = new Cypress;
}

CeleDriver::~CeleDriver(void)
{

}

bool CeleDriver::openUSB()
{
	return m_pCypress->open_usb();
}

bool CeleDriver::openStream()
{
	return m_pCypress->open_stream();
}

void CeleDriver::closeUSB()
{
	m_pCypress->close_usb();
}

void CeleDriver::closeStream()
{
	m_pCypress->close_stream();
}

bool CeleDriver::i2c_set(uint16_t reg,uint16_t value)
{
    return m_pCypress->USB_Set(CTRL_I2C_SET_REG,reg,value);
}

bool CeleDriver::i2c_get(uint16_t reg,uint16_t &value)
{
    return m_pCypress->USB_Get(CTRL_I2C_GET_REG,reg,value);
}

bool CeleDriver::mipi_set(uint16_t reg,uint16_t value)
{
    return m_pCypress->USB_Set(CTRL_MIPI_SET_REG,reg,value);
}

bool CeleDriver::mipi_get(uint16_t reg,uint16_t &value)
{
    return m_pCypress->USB_Get(CTRL_MIPI_GET_REG,reg,value);
}

bool CeleDriver::writeSerialNumber(std::string number)
{
	return m_pCypress->writeSerialNumber(number);
}

std::string CeleDriver::getSerialNumber()
{
	return m_pCypress->getSerialNumber();
}

std::string CeleDriver::getFirmwareVersion()
{
	return m_pCypress->getFirmwareVersion();
}

std::string CeleDriver::getFirmwareDate()
{
	return m_pCypress->getFirmwareDate();
}

bool CeleDriver::getimage(std::vector<uint8_t> &image)
{
    return GetPicture(image);
}

bool CeleDriver::getSensorData(std::vector<uint8_t> &image, std::time_t& time_stamp_end, std::vector<IMU_Raw_Data>& imu_data)
{
	return GetPicture(image, time_stamp_end, imu_data);
}

void CeleDriver::clearData()
{
	ClearData();
}
