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

#include <string.h>
#include <stdio.h>
#include "cypress.h"
#include "celedriver.h"

CeleDriver::CeleDriver(void)
{
	m_pCypress = new Cypress;
	//ofTest.open("index.txt");
}

CeleDriver::~CeleDriver(void)
{

}

/*
*  @function: openUSB
*  @brief   : open usb
*  @input   : 
*  @output  :
*  @return  : true is successful to open usb
*/
bool CeleDriver::openUSB()
{
	return m_pCypress->openUSB();
}

/*
*  @function: openStream
*  @brief   : open usb stream
*  @input   :
*  @output  :
*  @return  : true is successful to open usb stream
*/
bool CeleDriver::openStream()
{
	return m_pCypress->openStream();
}

/*
*  @function: closeUSB
*  @brief   : close usb
*  @input   :
*  @output  :
*  @return  : true is successful to close usb
*/
void CeleDriver::closeUSB()
{
	m_pCypress->closeUSB();
}

/*
*  @function: closeStream
*  @brief   : close usb stream
*  @input   :
*  @output  :
*  @return  : true is successful to close usb stream
*/
void CeleDriver::closeStream()
{
	m_pCypress->closeStream();
}

/*
*  @function: i2cSet
*  @brief   : write sensor register by i2c
*  @input   : reg: register address
*             value: register value
*  @output  :
*  @return  : true is successful to write sensor register by i2c
*/
bool CeleDriver::i2cSet(uint16_t reg, uint16_t value)
{
    return m_pCypress->usbSet(CTRL_I2C_SET_REG, reg, value);
}

/*
*  @function: i2cGet
*  @brief   : read sensor register value by i2c (unused interface)
*  @input   : reg: register address
*             value: register value
*  @output  :
*  @return  : true is successful to read sensor register value by i2c
*/
bool CeleDriver::i2cGet(uint16_t reg, uint16_t &value)
{
    return m_pCypress->usbGet(CTRL_I2C_GET_REG, reg, value);
}

/*
*  @function: mipiSet
*  @brief   : write sensor register by mipi (unused interface)
*  @input   : reg: register address
*             value: register value
*  @output  :
*  @return  : true is successful to write sensor register by mipi
*/
bool CeleDriver::mipiSet(uint16_t reg, uint16_t value)
{
    return m_pCypress->usbSet(CTRL_MIPI_SET_REG, reg, value);
}

/*
*  @function: mipiGet
*  @brief   : read sensor register value by mipi (unused interface)
*  @input   : reg: register address
*             value: register value
*  @output  :
*  @return  : true is successful to read sensor register value by mipi
*/
bool CeleDriver::mipiGet(uint16_t reg, uint16_t &value)
{
    return m_pCypress->usbGet(CTRL_MIPI_GET_REG, reg, value);
}

/*
*  @function: writeSerialNumber
*  @brief   : write the serial number of sensor
*  @input   : number: sensor serial number
*  @output  :
*  @return  : true is successful to write sensor serial number
*/
bool CeleDriver::writeSerialNumber(std::string number)
{
	return m_pCypress->writeSerialNumber(number);
}

/*
*  @function: getSerialNumber
*  @brief   : get the serial number of sensor
*  @input   : 
*  @output  :
*  @return  : sensor serial number
*/
std::string CeleDriver::getSerialNumber()
{
	return m_pCypress->getSerialNumber();
}

/*
*  @function: getFirmwareVersion
*  @brief   : get firmware version
*  @input   :
*  @output  :
*  @return  : firmware version
*/
std::string CeleDriver::getFirmwareVersion()
{
	return m_pCypress->getFirmwareVersion();
}

/*
*  @function: getFirmwareDate
*  @brief   : get firmware date
*  @input   : 
*  @output  :
*  @return  : firmware date
*/
std::string CeleDriver::getFirmwareDate()
{
	return m_pCypress->getFirmwareDate();
}

bool CeleDriver::getSensorData(uint8_t* pData, uint32_t& length)
{
	if (g_uiPackageCount > 0)
	{
		g_mtxSensorData.lock();
		if (g_uiHeadIndex != g_uiTailIndex)
		{
			//printf("------ g_uiReadIndex = %d, g_uiPackageCount = %d\n", g_uiHeadIndex, g_uiPackageCount);
			//ofTest << g_uiHeadIndex << std::endl;
			g_PackageList[g_uiHeadIndex].getData(pData, length);

			if (g_bUsingIMUCallback)
			{
				g_IMURawDataList.clear();
			}
			g_PackageList[g_uiHeadIndex].m_vecIMUData.clear();
			//printf("------------- g_uiHeadIndex = %d-------------\n", g_uiHeadIndex);
			g_uiHeadIndex++;
			if (g_uiHeadIndex >= MAX_IMAGE_BUFFER_NUMBER)
				g_uiHeadIndex = 0;
			g_uiPackageCount--;
			//printf("------------- g_uiHeadIndex = %d, g_uiPackageCount = %d\n", g_uiHeadIndex, g_uiPackageCount);
			if (length > 0)
			{
				g_mtxSensorData.unlock();
				return true;
			}
		}
		else
		{
			//printf("------------- !!!!!!!!!!!!!!!!!!!!!!!!! -------------\n");
		}
		g_mtxSensorData.unlock();
	}
}

/*
*  @function: getSensorData
*  @brief   : get sensor data and the pc timestamp
*  @input   : buffer: sensor image data
*             length: buffer size
*             timestampEnd: the pc timestamp when the packet buffer was received
*  @output  :
*  @return  : true is successful to get sensor data
*/
bool CeleDriver::getSensorData(uint8_t* pData, uint32_t& length, std::time_t& timestampEnd, std::vector<IMURawData>& imuData)
{
	if (g_uiPackageCount > 0)
	{
		g_mtxSensorData.lock();
		if (g_uiHeadIndex != g_uiTailIndex)
		{
			//printf("------ g_uiReadIndex = %d, g_uiPackageCount = %d\n", g_uiHeadIndex, g_uiPackageCount);
			//ofTest << g_uiHeadIndex << std::endl;
			g_PackageList[g_uiHeadIndex].getData(pData, length);
			timestampEnd = g_PackageList[g_uiHeadIndex].m_lTimestampEnd;

			if (g_bUsingIMUCallback)
			{
				imuData = g_IMURawDataList;
				g_IMURawDataList.clear();
			}
			else
			{
				imuData = g_PackageList[g_uiHeadIndex].m_vecIMUData;
			}
			g_PackageList[g_uiHeadIndex].m_vecIMUData.clear();
			//printf("------------- g_uiHeadIndex = %d-------------\n", g_uiHeadIndex);
			g_uiHeadIndex++;
			if (g_uiHeadIndex >= MAX_IMAGE_BUFFER_NUMBER)
				g_uiHeadIndex = 0;
			g_uiPackageCount--;
			//printf("------------- g_uiHeadIndex = %d, g_uiPackageCount = %d\n", g_uiHeadIndex, g_uiPackageCount);
			if (length > 0)
			{
				g_mtxSensorData.unlock();
				return true;
			}
		}
		else
		{
			//printf("------------- !!!!!!!!!!!!!!!!!!!!!!!!! -------------\n");
		}
		g_mtxSensorData.unlock();
	}
	return false;
}

/*
*  @function: clearData
*  @brief   : clear data
*  @input   : 
*  @output  :
*  @return  :
*/
void CeleDriver::clearData()
{
	for (int i = 0; i < MAX_IMAGE_BUFFER_NUMBER; i++)
	{
		g_PackageList[i].clearData();
	}
	g_uiPackageCount = 0;
	g_uiTailIndex = 0;
	g_uiHeadIndex = 0;
}

uint16_t CeleDriver::getALSValue()
{
	return getALSValue();
}
