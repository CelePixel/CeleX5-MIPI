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

#ifndef USBINTERFACE_H
#define USBINTERFACE_H

#include <thread>

#ifdef __linux__
#include <pthread.h>
#else
#include <windows.h>
#include <process.h>
#endif
#include "bulktransfer.h"

#define MAX_URB_NUMBER       20
#define MAX_URB_NUMBER_IMU   20

class USBInterface
{
public:
	USBInterface();
    ~USBInterface();

	bool videoStart(void);
	void videoStop(void);

public:
    bool usbOpen(int vid, int pid, int transMode);
    void usbClose(void);
    bool usbControl(uint8_t requestType, uint8_t request, uint16_t value, uint16_t index, uint8_t *buffer, uint16_t len);

    bool start(void);
    void stop(void);

private:
    bool usbCheckDevice(libusb_device *dev, int vid, int pid, int transMode);
    bool usbGetInterface(int vid, int pid, int transMode);
    bool usbAllocBulkTransfer(void);
	bool usbAllocInterruptTransfer(void); //added by xiaoqin @2019.06.11 for receiving IMU data

	void worker(void);

private:
    libusb_device_handle* m_pDeviceHandle;

    libusb_transfer*      m_pBulkTransfer[MAX_URB_NUMBER];
    uint8_t               m_uiBulkBuffer[MAX_URB_NUMBER][MAX_ELEMENT_BUFFER_SIZE];

	libusb_transfer*      m_pInterruptTransfer[MAX_URB_NUMBER_IMU];
	uint8_t               m_uiInterruptBuffer[MAX_URB_NUMBER_IMU][32];


    std::vector<int>      m_vecInterfaceNumberList;
    int                   m_iConfigurationValue;
    int                   m_iVideoEndpointAddress;
	int                   m_iIMUEndpointAddress;
    int                   m_iVideoTransMode;
	bool                  m_bRunning;

	std::thread           m_threadProcessor;
};

#endif // USBINTERFACE_H
