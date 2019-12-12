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

#include <stdio.h>
#include <iostream>
#include <chrono>
#ifdef __linux__
#include <semaphore.h>
#include <string.h>
#endif // __linux__
#include "bulktransfer.h"

//std::ofstream ofTest;

static CPackage*  current_package = nullptr;
clock_t clock_begin = 0;
clock_t clock_end = 0;

bool        g_bTransferError = false;
bool        g_bUsingIMUCallback = false;
//
uint16_t    g_uiTailIndex = 0;
uint16_t    g_uiHeadIndex = 0;
uint32_t    g_uiPackageCount = 0;
CPackage    g_PackageList[MAX_IMAGE_BUFFER_NUMBER];
//
std::vector<IMURawData>    g_IMURawDataList;
std::mutex                 g_mtxSensorData;

bool submit_bulk_transfer(libusb_transfer *xfr)
{
    if (xfr)
    {
        if(libusb_submit_transfer(xfr) == LIBUSB_SUCCESS)
        {
            return true;
            // Error
        }
        libusb_free_transfer(xfr);
    }
    return false;
}

std::time_t getTimeStamp() 
{
	std::chrono::time_point<std::chrono::system_clock, std::chrono::milliseconds> tp = std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::system_clock::now());
	auto tmp = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch());
	std::time_t timestamp = tmp.count();
	//std::time_t timestamp = std::chrono::system_clock::to_time_t(tp);
	//cout << "     ---- time stamp =       " << timestamp << endl;
	return timestamp;
}

void generate_image(uint8_t *buffer, int length)
{
	if (current_package == nullptr)
	{
		current_package = &g_PackageList[g_uiTailIndex];
		//ofTest << "------- " << g_uiTailIndex << std::endl;
		current_package->clearData();
		if (g_uiPackageCount >= MAX_IMAGE_BUFFER_NUMBER)
		{
			g_uiPackageCount--;

			g_uiHeadIndex++;
			if (g_uiHeadIndex >= MAX_IMAGE_BUFFER_NUMBER)
				g_uiHeadIndex = 0;
			printf("------- generate_image: buffer is full! -------\n");
			//ofTest << "------- generate_image: buffer is full! -------" << std::endl;
		}
		//printf("--- g_uiPackageCount = %d, g_uiWriteIndex = %d\n", g_uiPackageCount, g_uiWriteIndex);
	}
	//
	if (current_package)
	{
		if (!g_bUsingIMUCallback)
		{
			if (buffer[7] == 1)
			{
				IMURawData imu_data;
				memcpy(imu_data.imuData, buffer + 8, 20);
				imu_data.timestamp = getTimeStamp();

				current_package->m_vecIMUData.push_back(imu_data);
			}
		}
		if (buffer[1] & 0x02)
		{
			current_package->m_lTimestampEnd = getTimeStamp();
			//cout << "------------ image time stamp = " << current_package->m_lTime_Stamp_End << endl;
			buffer[length] = *(buffer + 6);
			current_package->insert(buffer + buffer[0], length - buffer[0] + 1);
			//current_package->Insert(buffer + 6, 1);
			current_package->end();
			current_package = nullptr;

			g_uiTailIndex++;
			if (g_uiTailIndex >= MAX_IMAGE_BUFFER_NUMBER)
				g_uiTailIndex = 0;
			g_uiPackageCount++;
		}
		else
		{
			current_package->insert(buffer + buffer[0], length - buffer[0]);
		}
	}
}

void generate_imu_data(uint8_t *buffer, int length)
{
	if (buffer[0] == 0x01) //valid imu data
	{
		IMURawData imu_data;
		//printf("------ generate_imu_data: valid imu data!\n");
		memcpy(imu_data.imuData, buffer + 1, 20);
		imu_data.timestamp = getTimeStamp();
		
		//cout << "generate_imu_data: " << imu_data.time_stamp << endl;

		g_mtxSensorData.lock();
		g_IMURawDataList.push_back(imu_data);
		g_mtxSensorData.unlock();
		//std::cout << imu_data.timestamp << std::endl;
	}
	else
	{
		printf("generate_imu_data: invalid imu data!\n");
	}
}

void callbackUSBTransferCompleted(libusb_transfer *xfr)
{
    switch (xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
			//printf("xfr->actual_length= %d\r\n", xfr->actual_length);
            generate_image(xfr->buffer, xfr->actual_length);
            submit_bulk_transfer(xfr);
            break;

        case LIBUSB_TRANSFER_TIMED_OUT:
			printf("LIBUSB_TRANSFER_TIMED_OUT\r\n");
            break;

        case LIBUSB_TRANSFER_CANCELLED:
			printf("LIBUSB_TRANSFER_CANCELLED\r\n");
			break;

        case LIBUSB_TRANSFER_NO_DEVICE:
			printf("LIBUSB_TRANSFER_NO_DEVICE\r\n");
			break;

        case LIBUSB_TRANSFER_ERROR:
			g_bTransferError = true;
			printf("LIBUSB_TRANSFER_ERROR\r\n");
			break;

        case LIBUSB_TRANSFER_STALL:
			printf("LIBUSB_TRANSFER_STALL\r\n");
			break;

        case LIBUSB_TRANSFER_OVERFLOW:
			printf("LIBUSB_TRANSFER_OVERFLOW\r\n");
            break;
    }
}

void cbUSBInterruptTransferCompleted(libusb_transfer *xfr)
{
	switch (xfr->status)
	{
	case LIBUSB_TRANSFER_COMPLETED:
		//printf("xfr->actual_length= %d\r\n", xfr->actual_length);
		generate_imu_data(xfr->buffer, xfr->actual_length);
		submit_bulk_transfer(xfr);
		break;

	case LIBUSB_TRANSFER_TIMED_OUT:
		printf("LIBUSB_TRANSFER_TIMED_OUT\r\n");
		break;

	case LIBUSB_TRANSFER_CANCELLED:
		printf("LIBUSB_TRANSFER_CANCELLED\r\n");
		break;

	case LIBUSB_TRANSFER_NO_DEVICE:
		printf("LIBUSB_TRANSFER_NO_DEVICE\r\n");
		break;

	case LIBUSB_TRANSFER_ERROR:
		g_bTransferError = true;
		printf("LIBUSB_TRANSFER_ERROR\r\n");
		break;

	case LIBUSB_TRANSFER_STALL:
		printf("LIBUSB_TRANSFER_STALL\r\n");
		break;

	case LIBUSB_TRANSFER_OVERFLOW:
		printf("LIBUSB_TRANSFER_OVERFLOW\r\n");
		break;
	}
}

libusb_transfer *alloc_bulk_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer)
{
    if (device_handle)
    {
        libusb_transfer *xfr = libusb_alloc_transfer(0);
        if (xfr)
        {
            libusb_fill_bulk_transfer(xfr,
                          device_handle,
                          address, // Endpoint ID
                          buffer,
                          MAX_ELEMENT_BUFFER_SIZE,
                          callbackUSBTransferCompleted,
                          nullptr,
                          0
                          );
            if (submit_bulk_transfer(xfr) == true)
                return xfr;
        }
    }
    return nullptr;
}

libusb_transfer *alloc_interrupt_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer)
{
	if (device_handle)
	{
		libusb_transfer *xfr = libusb_alloc_transfer(0);
		if (xfr)
		{
			libusb_fill_interrupt_transfer(xfr,
				device_handle,
				address, // Endpoint ID
				buffer,
				32,
				cbUSBInterruptTransferCompleted,
				nullptr,
				0
			);
			if (submit_bulk_transfer(xfr) == true)
				return xfr;
		}
	}
	return nullptr;
}

void cancel_bulk_transfer(libusb_transfer *xfr)
{
    if (xfr)
    {
        libusb_cancel_transfer(xfr);
       // libusb_free_transfer(xfr);
    }
}
