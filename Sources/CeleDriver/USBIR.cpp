#include "USBIR.h"
#include <stdio.h>
#include <string.h>

#define USB_TIMEOUT     3000           //传输数据的时间延迟

USBIR::USBIR()
{
    device_handle = nullptr;
}

USBIR::~USBIR()
{

}

bool USBIR::usb_check_device(libusb_device *dev,int usb_vid,int usb_pid,int trans_mode)
{
    libusb_device_descriptor desc;
    if(libusb_get_device_descriptor(dev, &desc) == LIBUSB_SUCCESS )
    {
        if ( ( desc.idVendor == usb_vid ) && ( desc.idProduct == usb_pid ) )
        {
            for (int i = 0; i < desc.bNumConfigurations; i++)
            {
                struct libusb_config_descriptor *config;
                if ( libusb_get_config_descriptor(dev, i, &config) == LIBUSB_SUCCESS )
                {
                    bConfigurationValue    = config->bConfigurationValue;
                    for (int j = 0; j < config->bNumInterfaces; j++)
                    {
                        for ( int k = 0; k < config->interface[j].num_altsetting; k++)
                        {
                            if (config->interface[j].altsetting[k].bInterfaceClass == LIBUSB_CLASS_VIDEO )
                            {
                                InterfaceNumberList.push_back(config->interface[j].altsetting[k].bInterfaceNumber);
                                for ( int l = 0; l < config->interface[j].altsetting[k].bNumEndpoints; l++)
                                {
                                    if( ( config->interface[j].altsetting[k].endpoint[l].bmAttributes & 0x03 ) == trans_mode ) //找到端点
                                    {
                                        video_endpoint_address = config->interface[j].altsetting[k].endpoint[l].bEndpointAddress;
                                        libusb_free_config_descriptor(config);
                                        return true;
                                    }
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

bool USBIR::usb_GetInterface(int usb_vid, int usb_pid,int trans_mode)
{
    libusb_device **devs;
    ssize_t cnt = libusb_get_device_list(nullptr, &devs);

    for (ssize_t i = 0; i < cnt; i++ )
    {
        if( usb_check_device(devs[i], usb_vid,usb_pid ,trans_mode) == true )
        {
            libusb_free_device_list(devs, 1);
            return true;
        }
    }

    if ( cnt >= 0)
    {
        libusb_free_device_list(devs, 1);
    }
    return false;
}

bool USBIR::usb_open(int vid, int pid,int trans_mode)
{
    if( libusb_init(nullptr) == LIBUSB_SUCCESS )
    {
        InterfaceNumberList.clear();
        if (  usb_GetInterface(vid,pid,trans_mode) == true )
        {
            device_handle = libusb_open_device_with_vid_pid(nullptr, vid, pid);
            if ( device_handle )
            {
                 /* 进行设备的初始化
                 1.设置当前的设备使用的configuration,参数2是要使用配置描述符中的bConfigurationValue */
                libusb_set_configuration(device_handle,bConfigurationValue);
                size_t i = 0;
                for( ;i < InterfaceNumberList.size();i++ )
                {
                    int Ret = libusb_detach_kernel_driver(device_handle,InterfaceNumberList[i]);
                    if ( (Ret == LIBUSB_SUCCESS) || ( Ret == LIBUSB_ERROR_NOT_SUPPORTED ) || (Ret == LIBUSB_ERROR_NOT_FOUND ) )
                    {
                        if( libusb_claim_interface(device_handle, InterfaceNumberList[i]) != LIBUSB_SUCCESS )
                        {
                            break;
                        }
                    }
                }
                if ( i < InterfaceNumberList.size() )
                {
                    for( size_t j = 0; j < i; j++ )
                    {
                        libusb_release_interface(device_handle, InterfaceNumberList[j]);
                        libusb_attach_kernel_driver(device_handle, InterfaceNumberList[j]);
                    }

                } else
                {
                    video_trans_mode = trans_mode;
                    return true;
                }
                libusb_close(device_handle);
            }
        }
        libusb_exit(nullptr);
    }
    return false;
}

void USBIR::usb_close(void)
{
    if (device_handle)
    {
        for (size_t i = 0; i < InterfaceNumberList.size(); i++)
        {
            libusb_release_interface(device_handle, InterfaceNumberList[i]);
            libusb_attach_kernel_driver(device_handle, InterfaceNumberList[i]);
        }
        libusb_close(device_handle);
        libusb_exit(NULL);
    }
}

bool USBIR::usb_control(uint8_t request_type,uint8_t request, uint16_t wValue, uint16_t wIndex,uint8_t *buffer,uint16_t wLen)
{
    if( device_handle )
    {
        int ret = libusb_control_transfer(device_handle, request_type, request, wValue, wIndex, buffer,wLen, USB_TIMEOUT);
        if ( ret == wLen )
        {
            return true;
        }
        printf("Write return value = %d\r\n",ret);
    }
    return false;
}

bool USBIR::usb_read_data(unsigned char *data, int length,int *actual_length)
{
    if( device_handle )
    {
        int ret = 0;
        /* 读Image数据 */
        if ( video_trans_mode == LIBUSB_TRANSFER_TYPE_BULK )
        {
            ret = libusb_bulk_transfer(device_handle, video_endpoint_address, data, length,actual_length,0);
        } else
        {
            ret = libusb_interrupt_transfer(device_handle, video_endpoint_address, data, length,actual_length,USB_TIMEOUT);
        }
        if( ret == LIBUSB_SUCCESS )
        {
            return true;
        } else if ( ret == LIBUSB_ERROR_PIPE )
        {
            libusb_clear_halt(device_handle, video_endpoint_address);
        }
    }
    return false;
}


bool USBIR::usb_alloc_bulk_transfer(void)
{
    int i = 0;
    for( i = 0; i < MAX_URB_NUMBER; i++ )
    {
        bulk_transfer[i] = alloc_bulk_transfer(device_handle,video_endpoint_address,bulk_buffer[i]);
        if( bulk_transfer[i] == nullptr )
        {
            break;
        }
    }
    if ( i > 0 )
        return true;
    return false;
}

bool USBIR::Start(void)
{
    if ( video_start() == true )
    {
        if ( Init() == true )
        {
            if ( usb_alloc_bulk_transfer() == true )
            {
                return true;
            }
            Exit();
        }
        video_stop();
    }
    return false;
}

void USBIR::Stop(void)
{
    for( int i = 0; i < MAX_URB_NUMBER; i++ )
    {
        cancel_bulk_transfer(bulk_transfer[i]);
    }
    video_stop();
    Exit();

}

bool USBIR::Run(void)
{
    if(libusb_handle_events(NULL) != LIBUSB_SUCCESS) return false;
    //usb_alloc_bulk_transfer();
    return true;
}
