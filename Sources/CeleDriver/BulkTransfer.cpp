#include <stdio.h>
#include "BulkTransfer.h"
#ifdef __linux__
#include <semaphore.h>
#endif // __linux__

//#define _USING_NEW_RW_BUFFER_

#ifdef _USING_NEW_RW_BUFFER_
typedef struct PackInfo
{
	unsigned char* data;
	uint32_t       length;
} PackInfo;
#define MAX_PACKAGE_COUNT 50
std::vector<unsigned char> vec_packge_buffer;
PackInfo package_list[MAX_PACKAGE_COUNT];
static bool bNew = true;
#else
#define MAX_IMAGE_BUFFER_NUMBER 5
CPackage  image_list[MAX_IMAGE_BUFFER_NUMBER];
static CPackage*  current_package = nullptr;
#endif // _USING_NEW_RW_BUFFER_

static bool      bRunning = false;
static uint8_t   write_frame_index = 0;
static uint8_t   read_frame_index = 0;
static uint32_t  package_size = 0;
static uint32_t  package_count = 0;

#ifdef __linux__
    static sem_t     m_sem;
#else
	static HANDLE m_hEventHandle = nullptr;
#endif // __linux__

bool submit_bulk_transfer(libusb_transfer *xfr)
{
    if ( xfr )
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

#ifdef _USING_NEW_RW_BUFFER_

void generate_image(uint8_t *buffer, int length)
{
	if (bNew)
	{
		for (int i = 0; i < MAX_PACKAGE_COUNT; i++)
		{
			package_list[i].data = new unsigned char[1536001];
		}
		bNew = false;
	}
	//celex_generate_package(xfr->buffer, xfr->actual_length, cb_generate_package);

	if (package_size == 0)
		vec_packge_buffer.clear();
	int len = length - buffer[0];
	vec_packge_buffer.insert(vec_packge_buffer.end(), buffer + buffer[0], buffer + buffer[0] + len);
	package_size += len;
	if (buffer[1] & 0x02)
	{
		vec_packge_buffer.insert(vec_packge_buffer.end(), buffer + 6, buffer + 7);
		package_size += 1;
		if (*(buffer + 6) == 0 && package_size != 1536001)
		{
			package_size = 0;
			printf("--- generate error package = %d\n", vec_packge_buffer.size());
			return;
		}
		//printf("--- generate package = %d\n", vec_packge_buffer.size());
		memcpy(package_list[write_frame_index].data, vec_packge_buffer.data(), package_size > 1536001 ? 1536001 : package_size);
		package_list[write_frame_index].length = package_size;
		package_count++;
		if (package_count == MAX_PACKAGE_COUNT)
			printf("--- generate package is full = %d\n", package_count);
		write_frame_index++;
		if (write_frame_index == MAX_PACKAGE_COUNT)
			write_frame_index = 0;
		package_size = 0;
	}
	if (buffer[1] & 0x02)
	{
#ifdef __linux__
		sem_post(&m_sem);
#else
		SetEvent(m_hEventHandle);
#endif // __linux__
		}
}

bool GetPicture(vector<uint8_t> &Image)
{
	if (bRunning == true)
	{
#ifdef __linux__
		int Ret = sem_wait(&m_sem);
#else
		DWORD Ret = WaitForSingleObject(m_hEventHandle, INFINITE);
#endif // __linux__
		//if (Ret == 0)
		if (package_count > 0)
		{
			Image.insert(Image.end(), package_list[read_frame_index].data, package_list[read_frame_index].data + package_list[read_frame_index].length);
			read_frame_index++;
			package_count--;
			if (read_frame_index >= MAX_PACKAGE_COUNT)
				read_frame_index = 0;
			if (Image.size())
				return true;
		}
	}
	return false;
}

void ClearData()
{
	package_count = 0;
	write_frame_index = 0;
	read_frame_index = 0;
}

#else

void generate_image(uint8_t *buffer, int length)
{
	if (current_package == nullptr)
	{
		package_size = 0;
		current_package = &image_list[write_frame_index];
		write_frame_index++;
		if (write_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
			write_frame_index = 0;
		if (write_frame_index == read_frame_index)
		{
			read_frame_index++;
			if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
				read_frame_index = 0;
			//printf("-------- generate_image: buffer is full! --------");
		}
	}
	if (current_package)
		current_package->Insert(buffer + buffer[0], length - buffer[0]);
	package_size += (length - 12);
	if (buffer[1] & 0x02)
	{
		/*if (package_size != 357000)
			printf("-------- package size = %d\n", package_size);*/
		if (current_package)
		{
			current_package->Insert(buffer + 6, 1);
			current_package->End();
			current_package = nullptr;
#ifdef __linux__
			sem_post(&m_sem);
#else
			SetEvent(m_hEventHandle);
#endif // __linux__
		}
	}
}

bool GetPicture(vector<uint8_t> &Image)
{
	if (bRunning == true)
	{
#ifdef __linux__
		int Ret = sem_wait(&m_sem);
#else
		DWORD Ret = WaitForSingleObject(m_hEventHandle, INFINITE);
#endif // __linux__
		if (Ret == 0)
		{
			image_list[read_frame_index].GetImage(Image);
			read_frame_index++;
			if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
				read_frame_index = 0;
			if (Image.size())
				return true;
		}
	}
	return false;
}

bool GetPackage(unsigned char* buffer, uint32_t* length)
{
	if (bRunning == true)
	{
#ifdef __linux__
		int Ret = sem_wait(&m_sem);
#else
		DWORD Ret = WaitForSingleObject(m_hEventHandle, INFINITE);
#endif // __linux__
		if (Ret == 0)
		{
			image_list[read_frame_index].GetPackage(buffer, length);
			read_frame_index++;
			if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
				read_frame_index = 0;
			if (*length > 0)
				return true;
		}
	}
	return false;
}

void ClearData()
{
	for (int i = 0; i < MAX_IMAGE_BUFFER_NUMBER; i++)
	{
		//image_list[i].ClearData();
		package_count = 0;
		write_frame_index = 0;
		read_frame_index = 0;
	}
}

#endif // _USING_NEW_RW_BUFFER_


void callbackUSBTransferComplete(libusb_transfer *xfr)
{
    switch(xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
            // Success here, data transfered are inside
            // xfr->buffer
            // and the length is
            // xfr->actual_length
			/*if (xfr->actual_length != 42684 && xfr->actual_length != 15636)
				printf("xfr->actual_length= %d\r\n",xfr->actual_length);*/
            generate_image(xfr->buffer, xfr->actual_length);
          //  usb_alloc_bulk_transfer();
         // usb_submit_bulk_transfer();
            submit_bulk_transfer(xfr);
            break;
        case LIBUSB_TRANSFER_TIMED_OUT:
			printf("LIBUSB_TRANSFER_TIMED_OUT\r\n");
            break;
        case LIBUSB_TRANSFER_CANCELLED:
        case LIBUSB_TRANSFER_NO_DEVICE:
        case LIBUSB_TRANSFER_ERROR:
        case LIBUSB_TRANSFER_STALL:
        case LIBUSB_TRANSFER_OVERFLOW:
            // Various type of errors here
            break;
    }
}

libusb_transfer *alloc_bulk_transfer(libusb_device_handle *device_handle,uint8_t address,uint8_t *buffer)
{
    if ( device_handle )
    {
        libusb_transfer *xfr = libusb_alloc_transfer(0);
        if ( xfr )
        {
            libusb_fill_bulk_transfer(xfr,
                          device_handle,
                          address, // Endpoint ID
                          buffer,
                          MAX_ELEMENT_BUFFER_SIZE,
                          callbackUSBTransferComplete,
                          nullptr,
                          0
                          );
            if ( submit_bulk_transfer(xfr) == true )
                return xfr;
        }
    }
    return nullptr;
}

bool Init(void)
{
#ifdef __linux__
    if ( sem_init(&m_sem,0,0) == 0 )
    {
#else
    m_hEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_hEventHandle)
    {
#endif
        bRunning = true;
        return true;
    }
    return false;
}

void Exit(void)
{
    bRunning = false;
#ifdef __linux__
    sem_destroy(&m_sem);
#else
    if(m_hEventHandle)
    {
		CloseHandle(m_hEventHandle);
		m_hEventHandle = nullptr;
    }
#endif
}

void cancel_bulk_transfer(libusb_transfer *xfr)
{
    if ( xfr )
    {
        libusb_cancel_transfer(xfr);
       // libusb_free_transfer(xfr);
    }
}
