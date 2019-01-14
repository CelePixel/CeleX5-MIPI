#ifndef USBIR_H
#define USBIR_H

#ifdef __linux__
#include "libusb.h"//libusb-0.1.12库的头文件
#include "BulkTransfer.h"
#endif 
#include "VideoThread.h"

#define MAX_URB_NUMBER   20

class USBIR:public CVideoThread
{
public:
     USBIR();
    ~USBIR();
public:
    bool usb_open(int vid, int pid,int trans_mode);
    void usb_close(void);
    bool usb_control(uint8_t request_type,uint8_t request, uint16_t wValue, uint16_t wIndex,uint8_t *buffer,uint16_t wLen);
    bool usb_read_data(unsigned char *data, int length,int *actual_length);

public:
    bool Start(void);
    void Stop(void);

protected:
    bool Run(void);

private:
    bool usb_check_device(libusb_device *dev,int usb_vid,int usb_pid,int trans_mode);
    bool usb_GetInterface(int usb_vid, int usb_pid,int trans_mode);
    bool usb_alloc_bulk_transfer(void);
    bool usb_submit_bulk_transfer(void);

private:
    libusb_device_handle *device_handle;
    libusb_transfer      *bulk_transfer[MAX_URB_NUMBER];
    uint8_t               bulk_buffer[MAX_URB_NUMBER][MAX_ELEMENT_BUFFER_SIZE];
    vector<int>           InterfaceNumberList;
    int                   bConfigurationValue;
    int                   video_endpoint_address;
    int                   video_trans_mode;
};

#endif // USBIR_H
