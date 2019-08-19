#ifndef USBIR_H
#define USBIR_H

#ifdef __linux__
#include <pthread.h>
//#include "libusb.h"//libusb-0.1.12库的头文件
#include "BulkTransfer.h"
#else
#include <windows.h>
#include <process.h>
#endif
#include "BulkTransfer.h"

#define MAX_URB_NUMBER       20
#define MAX_URB_NUMBER_IMU   20

class USBIR
{
public:
     USBIR();
    ~USBIR();

	bool video_start(void);
	void video_stop(void);

public:
    bool usb_open(int vid, int pid, int trans_mode);
    void usb_close(void);
    bool usb_control(uint8_t request_type, uint8_t request, uint16_t wValue, uint16_t wIndex, uint8_t *buffer, uint16_t wLen);
    bool usb_read_data(unsigned char *data, int length, int *actual_length);

    bool start(void);
    void stop(void);

private:
    bool usb_check_device(libusb_device *dev, int usb_vid, int usb_pid, int trans_mode);
    bool usb_GetInterface(int usb_vid, int usb_pid, int trans_mode);
    bool usb_alloc_bulk_transfer(void);
	bool usb_alloc_interrupt_transfer(void); //added by xiaoqin @2019.06.11 for receiving IMU data

#ifdef __linux__
	static void* staticThreadFunc(void* args);
#else
	static DWORD staticThreadFunc(LPVOID args);
#endif
	void Worker(void);

private:
    libusb_device_handle *device_handle;

    libusb_transfer      *bulk_transfer[MAX_URB_NUMBER];
    uint8_t               bulk_buffer[MAX_URB_NUMBER][MAX_ELEMENT_BUFFER_SIZE];

	libusb_transfer      *interrupt_transfer[MAX_URB_NUMBER_IMU];
	uint8_t               interrupt_buffer[MAX_URB_NUMBER_IMU][32];


    std::vector<int>      InterfaceNumberList;
    int                   bConfigurationValue;
    int                   video_endpoint_address;
	int                   imu_endpoint_address;
    int                   video_trans_mode;
	clock_t               clock_begin;
	clock_t               clock_end;

	bool                  m_bRunning;

#ifdef __linux__
	pthread_t             m_threadID;
#else
	HANDLE                m_handle;
#endif
};

#endif // USBIR_H
