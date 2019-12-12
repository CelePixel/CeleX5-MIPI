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
#include <string.h>
#include "bulktransfer.h"
#include "usbinterface.h"

#define USB_TIMEOUT     3000

USBInterface::USBInterface()
{
	m_pDeviceHandle = nullptr;
	m_bRunning = false;
}

USBInterface::~USBInterface()
{

}

/*
*  @function: videoStart
*  @brief   : start USB transfer thread
*  @input   :
*  @output  :
*  @return  :
*/
bool USBInterface::videoStart(void)
{
	m_bRunning = true;
	m_threadProcessor = std::thread(&USBInterface::worker, this);
	m_threadProcessor.detach();

	return true;
}

/*
*  @function: videoStop
*  @brief   : stop USB transfer thread
*  @input   : 
*  @output  :
*  @return  :
*/
void USBInterface::videoStop(void)
{
	m_bRunning = false;
	printf("stop thread!\r\n");
}

/*
*  @function: worker
*  @brief   : thread function
*  @input   : 
*  @output  :
*  @return  : true is successful to find USB device
*/
void USBInterface::worker(void)
{
	while (m_bRunning)
	{
		if (g_bTransferError)
		{
			//close video stream
			usbControl(0x41, 0x88, 0x00, 0x00, nullptr, 0);
			for (int i = 0; i < MAX_URB_NUMBER; i++)
			{
				cancel_bulk_transfer(m_pBulkTransfer[i]);
			}
			//close usb device
			usbClose();

			//open usb device
			usbOpen(0x04b4, 0x00f1, LIBUSB_TRANSFER_TYPE_BULK);
			//open video stream
			uint8_t video_start[] = { 0x00, 0x00,0x01,0x02,0x0A,0x8B,0x02,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x20,0x1C,0x00, 0x00,0x90,0x00, 0x00 };
			if (usbControl(0x21, 0x01, 0x201, 0x0, video_start, sizeof(video_start)) == true)
			{
				usbAllocBulkTransfer();
			}
			g_bTransferError = false;
		}
		if (libusb_handle_events(NULL) != LIBUSB_SUCCESS)
		{
			break;
		}	
	}
	printf("exit thread!\r\n");
}

/*
*  @function: usbCheckDevice
*  @brief   :
*  @input   : dev: libusb_device struct
*             vid: USB device vendor ID
*             pid: USB device product ID
*             transMode: endpoint
*  @output  :
*  @return  : true is successful to find USB device
*/
bool USBInterface::usbCheckDevice(libusb_device *dev, int vid, int pid, int transMode)
{
    libusb_device_descriptor desc;
    if (libusb_get_device_descriptor(dev, &desc) == LIBUSB_SUCCESS)
    {
        if ((desc.idVendor == vid) && (desc.idProduct == pid))
        {
            for (int i = 0; i < desc.bNumConfigurations; i++)
            {
                struct libusb_config_descriptor *config;
                if (libusb_get_config_descriptor(dev, i, &config) == LIBUSB_SUCCESS)
                {
                    m_iConfigurationValue = config->bConfigurationValue;
                    for (int j = 0; j < config->bNumInterfaces; j++)
                    {
                        for (int k = 0; k < config->interface[j].num_altsetting; k++)
                        {
                            if (config->interface[j].altsetting[k].bInterfaceClass == LIBUSB_CLASS_VIDEO )
                            {
								m_vecInterfaceNumberList.push_back(config->interface[j].altsetting[k].bInterfaceNumber);
								bool bFindVideo = false, bFindIMU = false;
                                for (int l = 0; l < config->interface[j].altsetting[k].bNumEndpoints; l++)
                                {
                                    if (!bFindVideo && (config->interface[j].altsetting[k].endpoint[l].bmAttributes & 0x03) == transMode) //find endpoint: video
                                    {
										m_iVideoEndpointAddress = config->interface[j].altsetting[k].endpoint[l].bEndpointAddress;
                                        //libusb_free_config_descriptor(config);
										//return true;
										bFindVideo = true;
										printf("------------------- find video!\n");
                                    }
									if (!bFindIMU && (config->interface[j].altsetting[k].endpoint[l].bmAttributes & 0x03) == LIBUSB_TRANSFER_TYPE_INTERRUPT) //find endpoint: imu
									{
										m_iIMUEndpointAddress = config->interface[j].altsetting[k].endpoint[l].bEndpointAddress;
										//libusb_free_config_descriptor(config);
										//return true;
										printf("------------------- find imu!\n");
										bFindIMU = true;
										g_bUsingIMUCallback = true;
									}
                                }
								if (bFindVideo/* && bFindIMU*/)
								{
									libusb_free_config_descriptor(config);
									return true;
								}
                            }
                        }
                    }
                    libusb_free_config_descriptor(config);
                }
            }
        }
    }
    return false;
}

/*
*  @function: usbGetInterface
*  @brief   : 
*  @input   : vid: USB device vendor ID
*             pid: USB device product ID
*             transMode: endpoint
*  @output  :
*  @return  : true is successful to open USB device
*/
bool USBInterface::usbGetInterface(int vid, int pid, int transMode)
{
    libusb_device **devs;
    ssize_t cnt = libusb_get_device_list(nullptr, &devs);

    for (ssize_t i = 0; i < cnt; i++)
    {
        if (usbCheckDevice(devs[i], vid, pid, transMode) == true)
        {
            libusb_free_device_list(devs, 1);
            return true;
        }
    }
    if (cnt >= 0)
    {
        libusb_free_device_list(devs, 1);
    }
    return false;
}

/*
*  @function: usbOpen
*  @brief   : open USB device by vid and pid
*  @input   : vid: USB device vendor ID
*             pid: USB device product ID
*             transMode: endpoint
*  @output  :
*  @return  : true is successful to open USB device
*/
bool USBInterface::usbOpen(int vid, int pid, int transMode)
{
    if (libusb_init(nullptr) == LIBUSB_SUCCESS)
    {
		m_vecInterfaceNumberList.clear();
        if (usbGetInterface(vid, pid, transMode) == true)
        {
			m_pDeviceHandle = libusb_open_device_with_vid_pid(nullptr, vid, pid);
            if (m_pDeviceHandle)
            {
                libusb_set_configuration(m_pDeviceHandle, m_iConfigurationValue);
                size_t i = 0;
                for ( ; i < m_vecInterfaceNumberList.size(); i++)
                {
                    int Ret = libusb_detach_kernel_driver(m_pDeviceHandle, m_vecInterfaceNumberList[i]);
                    if ((Ret == LIBUSB_SUCCESS) || (Ret == LIBUSB_ERROR_NOT_SUPPORTED) || (Ret == LIBUSB_ERROR_NOT_FOUND))
                    {
                        if (libusb_claim_interface(m_pDeviceHandle, m_vecInterfaceNumberList[i]) != LIBUSB_SUCCESS)
                        {
                            break;
                        }
                    }
                }
                if (i < m_vecInterfaceNumberList.size())
                {
                    for (size_t j = 0; j < i; j++)
                    {
                        libusb_release_interface(m_pDeviceHandle, m_vecInterfaceNumberList[j]);
                        libusb_attach_kernel_driver(m_pDeviceHandle, m_vecInterfaceNumberList[j]);
                    }
                }
				else
                {
					m_iVideoTransMode = transMode;
                    return true;
                }
                libusb_close(m_pDeviceHandle);
            }
        }
        libusb_exit(nullptr);
    }
    return false;
}

/*
*  @function: usbClose
*  @brief   : close USB device
*  @input   :
*  @output  :
*  @return  :
*/
void USBInterface::usbClose(void)
{
    if (m_pDeviceHandle)
    {
        for (size_t i = 0; i < m_vecInterfaceNumberList.size(); i++)
        {
            libusb_release_interface(m_pDeviceHandle, m_vecInterfaceNumberList[i]);
            libusb_attach_kernel_driver(m_pDeviceHandle, m_vecInterfaceNumberList[i]);
        }
        libusb_close(m_pDeviceHandle);
        libusb_exit(NULL);
    }
}

/*
*  @function: usbControl
*  @brief   : perform a USB control transfer
*  @input   : requestType: the request type field for the setup packet
*             request: the request field for the setup packet
*             value: the value field for the setup packet
*             index: the index field for the setup packet
*             buffer: a suitably-sized data buffer for either input or output (depending on direction bits within bmRequestType)
*             len: the length field for the setup packet. The data buffer should be at least this size.
*  @output  :
*  @return  : true is successful to write sensor serial number
*/
bool USBInterface::usbControl(uint8_t requestType, uint8_t request, uint16_t wValue, uint16_t index, uint8_t *buffer, uint16_t len)
{
    if (m_pDeviceHandle)
    {
        int ret = libusb_control_transfer(m_pDeviceHandle, requestType, request, wValue, index, buffer, len, USB_TIMEOUT);
        if (ret == len)
        {
            return true;
        }
        printf("Write return value = %d\r\n", ret);
    }
    return false;
}

/*
*  @function: usbAllocBulkTransfer
*  @brief   : alloc USB bulk transfer
*  @input   :
*  @output  :
*  @return  : true is successful to alloc USB bulk transfer
*/
bool USBInterface::usbAllocBulkTransfer(void)
{
    int i = 0;
    for (i = 0; i < MAX_URB_NUMBER; i++)
    {
		m_pBulkTransfer[i] = alloc_bulk_transfer(m_pDeviceHandle, m_iVideoEndpointAddress, m_uiBulkBuffer[i]);
        if (nullptr == m_pBulkTransfer[i])
        {
            break;
        }
    }
    if (i > 0)
        return true;
    return false;
}

/*
*  @function: usbAllocInterruptTransfer
*  @brief   : alloc USB interrupt transfer
*  @input   :
*  @output  :
*  @return  : true is successful to alloc USB interrupt transfer
*/
bool USBInterface::usbAllocInterruptTransfer(void)
{
	int i = 0;
	for (i = 0; i < MAX_URB_NUMBER_IMU; i++)
	{
		m_pInterruptTransfer[i] = alloc_interrupt_transfer(m_pDeviceHandle, m_iIMUEndpointAddress, m_uiInterruptBuffer[i]);
		if (nullptr == m_pInterruptTransfer[i])
		{
			printf("usb_alloc_interrupt_transfer failed!\n");
			break;
		}
	}
	if (i > 0)
		return true;
	return false;
}

/*
*  @function: start
*  @brief   : alloc USB transfer and start USB transfer thread
*  @input   :
*  @output  :
*  @return  :
*/
bool USBInterface::start(void)
{
    if (videoStart() == true)
    {
		bool bSucceed1 = false, bSucceed2 = false;
		if (usbAllocBulkTransfer() == true)
		{
			bSucceed1 = true;
			printf("usb_alloc_bulk_transfer was successful!\n");
			//return true;
		}
		//added by xiaoqin @2019.06.11 for receiving IMU data
		if (g_bUsingIMUCallback)
		{
			if (usbAllocInterruptTransfer() == true)
			{
				bSucceed2 = true;
				printf("usb_alloc_interrupt_transfer was successful!\n");
				//return true;
			}
		}
		if (bSucceed1)
		{
			return true;
		}
        videoStop();
    }
    return false;
}

/*
*  @function: stop
*  @brief   : stop USB transfer and exit USB transfer thread
*  @input   : 
*  @output  :
*  @return  : 
*/
void USBInterface::stop(void)
{
    for (int i = 0; i < MAX_URB_NUMBER; i++)
    {
		if (m_pBulkTransfer[i])
			libusb_cancel_transfer(m_pBulkTransfer[i]);
    }
	if (g_bUsingIMUCallback)
	{
		for (int i = 0; i < MAX_URB_NUMBER_IMU; i++)
		{
			if (m_pInterruptTransfer[i])
				libusb_cancel_transfer(m_pInterruptTransfer[i]);
		}
	}
    videoStop();
}
