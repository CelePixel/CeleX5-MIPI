#include <stdio.h>
#include "BulkTransfer.h"
#ifdef __linux__
#include <semaphore.h>
#include <string.h>
#endif // __linux__

#include <iostream>
#include <chrono>

libcelex_transfer_cb_fn cb_generate_package = nullptr;

#define MAX_IMAGE_BUFFER_NUMBER    20
CPackage  image_list[MAX_IMAGE_BUFFER_NUMBER];
static CPackage*  current_package = nullptr;

static bool      bRunning = false;
static uint8_t   write_frame_index = 0;
static uint8_t   current_write_frame_index = 0;
static uint8_t   read_frame_index = 0;
static uint32_t  package_count = 0;

clock_t clock_begin = 0;
clock_t clock_end = 0;
bool    g_has_imu_data = true;

#ifdef __linux__
    static sem_t     m_sem;
#else
	static HANDLE m_hEventHandle = nullptr;
#endif // __linux__

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

//void generate_image(uint8_t *buffer, int length)
//{
//	if (current_package == nullptr)
//	{
//		package_count++;
//		package_size = 0;
//		current_package = &image_list[write_frame_index];
//		write_frame_index++;
//		if (write_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
//			write_frame_index = 0;
//		if (write_frame_index == read_frame_index)
//		{
//			read_frame_index++;
//			if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
//				read_frame_index = 0;
//			//printf("-------- generate_image: buffer is full! --------");
//		}
//		current_package->ClearData();
//	}
//	if (current_package)
//		current_package->Insert(buffer + buffer[0], length - buffer[0]);
//	package_size += (length - 12);
//	if (buffer[1] & 0x02)
//	{
//#ifdef _WIN32
//		current_package->m_lTime_Stamp1 = 0;
//		current_package->m_lTime_Stamp2 = clock();
//#else		
//		clock_gettime(CLOCK_REALTIME, &time_real);
//		current_package->m_lTime_Stamp1 = time_real.tv_sec;
//		current_package->m_lTime_Stamp2 = time_real.tv_nsec;
//#endif
//		if (current_package)
//		{
//			current_package->Insert(buffer + 6, 1);
//			current_package->End();
//			current_package = nullptr;
//#ifdef __linux__
//			sem_post(&m_sem);
//#else
//			SetEvent(m_hEventHandle);
//#endif // __linux__
//		}
//	}
//}

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
		package_count++;
		current_package = &image_list[write_frame_index];
		current_write_frame_index = write_frame_index;
		write_frame_index++;
		if (write_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
			write_frame_index = 0;

		//if (current_package->Size() > 0)
		{
			//current_package->ClearData();
			//package_count--;

			//printf("package_count = %d:----------------\n", package_count);
		}
		if (write_frame_index == read_frame_index)
		{
			read_frame_index++;
			if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
				read_frame_index = 0;

			//printf("------- package_count = %d:-------\n", package_count);
			//printf("------- generate_image: buffer is full! -------");
		}
		//printf("---------------- package_count = %d:----------------\n", package_count);
	}
	if (current_package)
		current_package->Insert(buffer + buffer[0], length - buffer[0]);
	if (g_has_imu_data && buffer[7] == 1)
	{
		IMU_Raw_Data imu_data;
		memcpy(imu_data.imu_data, buffer + 7, 21);
		imu_data.time_stamp = getTimeStamp();

//#ifdef _WIN32
//		imu_data.time_stamp1 = 0;
//		imu_data.time_stamp2 = clock();
//#else		
//		clock_gettime(CLOCK_REALTIME, &time_real);
//		imu_data.time_stamp1 = time_real.tv_sec;
//		imu_data.time_stamp2 = time_real.tv_nsec;
//#endif
		current_package->m_vecIMUData.push_back(imu_data);
	}
	//
	if (buffer[1] & 0x02)
	{
		current_package->m_lTime_Stamp_End = getTimeStamp();
		//cout << "------------ image time stamp = " << current_package->m_lTime_Stamp_End << endl;
		if (current_package)
		{
			current_package->Insert(buffer + 6, 1);
			current_package->End();
			current_package = nullptr;

			package_count++;
//#ifdef __linux__
//			sem_post(&m_sem);
//#else
//			SetEvent(m_hEventHandle);
//#endif // __linux__
		}
	}
}

bool GetPicture(std::vector<uint8_t> &Image)
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
			package_count--;
			if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
				read_frame_index = 0;
			if (Image.size())
				return true;
		}
	}
	return false;
}

bool GetPicture(std::vector<uint8_t> &Image, long& time_stamp1, long& time_stamp2)
{
	if (bRunning == true)
	{
//#ifdef __linux__
//		int Ret = sem_wait(&m_sem);
//#else
//		DWORD Ret = WaitForSingleObject(m_hEventHandle, INFINITE);
//#endif // __linux__
		//if (Ret == 0)
		if (package_count > 0)
		{
			if (read_frame_index != current_write_frame_index)
			{
				image_list[read_frame_index].GetImage(Image);
				//printf("------------- read_frame_index = %d:-------------\n", read_frame_index);
				read_frame_index++;
				if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
					read_frame_index = 0;
				package_count--;
				if (Image.size())
				{
					return true;
				}
			}
			else
			{
				//printf("------------- read_frame_index = current_write_frame_index = %d -------------\n", read_frame_index);
			}
		}
	}
	return false;
}

bool GetPicture(std::vector<uint8_t> &Image, std::time_t& time_stamp_end, std::vector<IMU_Raw_Data>& imu_data)
{
	if (bRunning == true)
	{
//#ifdef __linux__
//		int Ret = sem_wait(&m_sem);
//#else
//		DWORD Ret = WaitForSingleObject(m_hEventHandle, INFINITE);
//#endif // __linux__
//if (Ret == 0)
		if (package_count > 0)
		{
			if (read_frame_index != current_write_frame_index)
			{
				image_list[read_frame_index].GetImage(Image);
				time_stamp_end = image_list[read_frame_index].m_lTime_Stamp_End;
				imu_data = image_list[read_frame_index].m_vecIMUData;
				image_list[read_frame_index].m_vecIMUData.clear();
				//printf("------------- read_frame_index = %d:-------------\n", read_frame_index);
				read_frame_index++;
				if (read_frame_index >= MAX_IMAGE_BUFFER_NUMBER)
					read_frame_index = 0;
				package_count--;
				if (Image.size())
				{
					return true;
				}
			}
			else
			{
				//printf("------------- read_frame_index = current_write_frame_index = %d -------------\n", read_frame_index);
			}
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

void callbackUSBTransferComplete(libusb_transfer *xfr)
{
    switch (xfr->status)
    {
        case LIBUSB_TRANSFER_COMPLETED:
            // Success here, data transfered are inside
            // xfr->buffer
            // and the length is
            // xfr->actual_length
			//printf("xfr->actual_length= %d\r\n",xfr->actual_length);
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

libusb_transfer *alloc_bulk_transfer(libusb_device_handle *device_handle, uint8_t address, uint8_t *buffer)
{
    if ( device_handle )
    {
        libusb_transfer *xfr = libusb_alloc_transfer(0);
        if (xfr)
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
            if (submit_bulk_transfer(xfr) == true)
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

void setCallback(libcelex_transfer_cb_fn callback)
{
	cb_generate_package = callback;
}
