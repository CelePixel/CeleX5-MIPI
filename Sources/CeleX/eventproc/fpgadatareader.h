/*
* Copyright (c) 2017-2018  CelePixel Technology Co. Ltd.  All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#ifndef FPGADATAREADER_H
#define FPGADATAREADER_H

#include "../include/celex4/celex4.h"
#include <stdint.h>

class FPGADataReader
{
public:
	enum IMUDATAType {
		NO_IMU_DATA = 0,
		GYROS_A_DATA = 1,
		GYROS_B_DATA = 2,
		ACC_A_DATA = 3,
		ACC_B_DATA = 4,
		GYROS_OFST_A_DATA = 5,
		GYROS_OFST_B_DATA = 6,
		ACC_OFST_A_DATA = 7,
		ACC_OFST_B_DATA = 8
	};
    static unsigned int getColumn(unsigned char data[EVENT_SIZE]);
    static unsigned int getRow(unsigned char data[EVENT_SIZE]);
    static unsigned int getTimeStamp(unsigned char data[EVENT_SIZE]);
    static unsigned int getBrightness(unsigned char data[EVENT_SIZE]);
    static unsigned int getCurrentRow() { return s_uiCurrentRow;    }
	static unsigned int getLastRow() { return s_uiLastRow; }
    static unsigned int getTFromFPGA()  { return s_uiTFromFPGA;     }
	static unsigned int getLastTFromFPGA() { return s_uiLastTFromFPGA; }
    static unsigned int getMapT()       { return s_uiMapT;          }
    static unsigned int getEventType()  { return s_uiEventType; }
    static unsigned int getSpecialEventType() { return s_uiSpecialEventType; }

	static void setEventCounter(uint32_t counter) { s_uiEventTCounter = counter; }

    //check event type
    static bool isForcefirePixel(unsigned char data[EVENT_SIZE]); // column event from force fire, carrying ADC
    static bool isColumnEvent(unsigned char data[EVENT_SIZE]);
    static bool isRowEvent(unsigned char data[EVENT_SIZE]);
    static bool isSpecialEvent(unsigned char data[EVENT_SIZE]);
	static unsigned int MapTime(unsigned char data[EVENT_SIZE]);
	//IMU data
	static IMUDATAType isIMUData(unsigned char data[4]);
	static int16_t getIMU_X(unsigned char data[4]);
	static int16_t getIMU_Y(unsigned char data[4]);
	static int16_t getIMU_Z(unsigned char data[4]);
	static uint16_t getIMU_T(unsigned char data[4]);
	static IMUData getIMUData() { return s_IMUData; }
    
private:
    static unsigned int  s_uiCurrentRow;
	static unsigned int  s_uiLastRow;
    static unsigned int  s_uiTFromFPGA;
	static unsigned int  s_uiLastTFromFPGA;
    static unsigned int  s_uiMapT;
    static unsigned int  s_uiEventType;
    static unsigned int  s_uiSpecialEventType;
	//IMU data
	static int16_t       s_iLowGYROS_Y;
	static uint16_t      s_uiHighGYROS_T;
	static IMUData       s_IMUData;
	static uint32_t      s_uiEventTCounter;
};

#endif // FPGADATAREADER_H
