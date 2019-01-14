#ifndef __BULK_TRANSFER_H__
#define __BULK_TRANSFER_H__

#include "libusb.h"
#include "Package.h"

bool Init(void);
void Exit(void);
libusb_transfer *alloc_bulk_transfer(libusb_device_handle *device_handle,uint8_t address,uint8_t *buffer);
void cancel_bulk_transfer(libusb_transfer *xfr);
bool GetPicture(vector<uint8_t> &Image);
bool GetPackage(unsigned char* buffer, uint32_t* length);
void ClearData();



#endif // __BULK_TRANSFER_H__
