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

#ifndef __BULK_TRANSFER_H__
#define __BULK_TRANSFER_H__

#include <mutex>
#include "package.h"
#include "../cx3driver/include/libusb.h"

#define MAX_IMAGE_BUFFER_NUMBER    100

//#include <fstream>
//extern std::ofstream ofTest;

extern bool        g_bTransferError;
extern bool        g_bUsingIMUCallback;
extern uint16_t    g_uiTailIndex;
extern uint16_t    g_uiHeadIndex;
extern uint32_t    g_uiPackageCount;
extern CPackage    g_PackageList[MAX_IMAGE_BUFFER_NUMBER];
//
extern std::vector<IMURawData>    g_IMURawDataList;
extern std::mutex                 g_mtxSensorData;

libusb_transfer *alloc_bulk_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer);
libusb_transfer *alloc_interrupt_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer); //added by xiaoqin @2019.06.11 for receiving IMU data
void cancel_bulk_transfer(libusb_transfer *xfr);

#endif // __BULK_TRANSFER_H__
