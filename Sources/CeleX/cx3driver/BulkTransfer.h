#ifndef __BULK_TRANSFER_H__
#define __BULK_TRANSFER_H__

#include "../cx3driver/include/libusb.h"
#include "Package.h"

extern bool    g_bTransfer_Error;
extern bool    g_bUsingIMUCallback;

bool Init(void);
void Exit(void);
libusb_transfer *alloc_bulk_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer);
libusb_transfer *alloc_interrupt_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer); //added by xiaoqin @2019.06.11 for receiving IMU data
void cancel_bulk_transfer(libusb_transfer *xfr);
bool GetPicture(std::vector<uint8_t> &Image);
bool GetPicture(std::vector<uint8_t> &Image, std::time_t& time_stamp_end, std::vector<IMU_Raw_Data>& imu_data);
void ClearData();

#endif // __BULK_TRANSFER_H__
