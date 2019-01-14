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

#include "celex5datareader.h"
#include <iostream>

using namespace std;

CeleX5DataReader::CeleX5DataReader()
	: m_uiRow(0)
	, m_uiLastRow(0)
	, m_uiColumn(0)
	, m_uiADC(0)
	, m_uiTFromFPGA(1)
	, m_uiLastTFromFPGA(0)
	, m_uiEventType(0)
{
}

CeleX5DataReader::~CeleX5DataReader()
{
}

//Byte 0: {1'b1, row[6:0]}
//Byte 1: {1'b1, row[9:7], time[4:1]}
//Byte 2: {1'b1, time[11:5]}
//Byte 3: {1'b10, time[17:12]}
bool CeleX5DataReader::isRowEvent(unsigned char data[EVENT_SIZE])
{
	return ((unsigned char)data[0] & 0x80) > 0 && !isSpecialEvent(data);
}

bool CeleX5DataReader::isRowEventEx(unsigned char data[EVENT_SIZE])
{
	if (((unsigned char)data[0] & 0x80) > 0 &&
		((unsigned char)data[1] & 0x80) > 0 &&
		((unsigned char)data[2] & 0x80) > 0 &&
		((unsigned char)data[3] & 0xC0) == 0x80)
		return true;
	return false;
}

//Byte 0: {1'b0, col[6:0]}
//Byte 1: {1'b0, col[10:7], A[2:0]}
//Byte 2: {3'b0, A[9:3]}
//Byte 3: {2'b00, A[11:10], mode[2:0], 1'b0}
bool CeleX5DataReader::isColumnEvent(unsigned char data[EVENT_SIZE])
{
	bool result = ((static_cast<unsigned char>(data[0]) & 0x80) == 0);
	if (result)
		m_uiEventType = (unsigned int)(static_cast<unsigned char>(data[3]) & 0x0E) >> 1; //8'b0000 1110
	return result;
}

//Byte 0: {8'b11111111}
//Byte 1: {8'b11111111}
//Byte 2: {8'b11111111}
//Byte 3: {8'b11111111}
bool CeleX5DataReader::isSpecialEvent(unsigned char data[EVENT_SIZE])
{
	//unsigned char a = static_cast<unsigned char>(data[0]);
	//unsigned char b = static_cast<unsigned char>(data[1]);
	//unsigned char c = static_cast<unsigned char>(data[2]);
	//unsigned char d = static_cast<unsigned char>(data[3]);

	//return (a == 0xff && b == 0xff && c == 0xff);

	unsigned char a = (unsigned char)data[0];
	unsigned char b = (unsigned char)data[1];
	unsigned char c = (unsigned char)data[2];
	unsigned char d = (unsigned char)data[3];

	bool result = (b == 0xff && c == 0xff && d == 0xff);

	if (result)
	{
		if ((0xFF & a) != 0xFD)
			return true;
		else
			return false;
		//cout << "------------------------------ " << int(0xFF & a) << endl;
	}
	return result;
}

//Byte 0: {1'b1, row[6:0]}
//Byte 1: {1'b1, row[9:7], time[4:1]}
//Byte 2: {1'b1, time[11:5]}
//Byte 3: {1'b10, time[17:12]}
void CeleX5DataReader::parseRowEvent(unsigned char data[EVENT_SIZE])
{
	m_uiLastRow = m_uiRow;
	m_uiRow = getRow(data);
	m_uiLastTFromFPGA = m_uiTFromFPGA;
	m_uiTFromFPGA = getTimeStamp(data);
}

//Byte 0: {1'b0, col[6:0]}
//Byte 1: {1'b0, col[10:7], A[2:0]}
//Byte 2: {3'b0, A[9:3]}
//Byte 3: {2'b00, A[11:10], mode[2:0], 1'b0}
void CeleX5DataReader::parseColumnEvent(unsigned char data[EVENT_SIZE])
{
	m_uiColumn = getColumn(data);
	m_uiADC = getADC(data);
}

void CeleX5DataReader::parseRowEventEx(unsigned char data[EVENT_SIZE])
{
	unsigned int byte1 = static_cast<unsigned char>(data[0]) & 0x70; //8'b0111 0000, need >> 4
	unsigned int byte2 = static_cast<unsigned char>(data[1]) & 0x70; //8'b0111 0000, need >> 4
	unsigned int byte3 = static_cast<unsigned char>(data[2]) & 0x70; //8'b0111 0000, need >> 4
	unsigned int byte4 = static_cast<unsigned char>(data[4]) & 0x10; //8'b0001 0000, need >> 4

	m_uiRow = (byte1 >> 4) + (byte2) + (byte3 << 4) + (byte4 << 8);
}

//-- CeleX4
//Byte 0: {1'b0, X[6:0]}
//Byte 1: {1'b0, C[0], X[8:7], A[3:0]}
//-- CeleX5
//Byte 0: {1'b0, col[6:0]}
//Byte 1: {1'b0, col[10:7]), A[2:0]}
unsigned int CeleX5DataReader::getColumn(unsigned char data[EVENT_SIZE])
{
	unsigned int low7bit = static_cast<unsigned char>(data[0]) & 0x7F;  //8'b0111 1111
	unsigned int high4bit = static_cast<unsigned char>(data[1]) & 0x78; //8'b0111 1000, need >> 3

	return low7bit + (high4bit << 4); // << (7 - 3)
}

//-- CeleX4
//Byte 0: {1'b1, Y[6:0]}
//Byte 1: {1'b1, Y[9:7], T[3:0]}
//-- CeleX5
//Byte 0: {1'b1, row[6:0]}
//Byte 1: {1'b1, row[9:7], time[4:1]}
unsigned int CeleX5DataReader::getRow(unsigned char data[EVENT_SIZE])
{
	unsigned int low7bit = static_cast<unsigned char>(data[0]) & 0x7F;  //8'b0111 1111
	unsigned int high3bit = static_cast<unsigned char>(data[1]) & 0x70; //8'b0111 0000, need >> 4
	return low7bit + (high3bit << 3); // << (7 - 4)
}

//-- CeleX4
//Byte 1: {1'b1, Y[9,7], T[3:0]}
//Byte 2: {1'b1, T[10:4]}
//Byte 3: {1'b1, T[17:11]}
//-- CeleX5
//Byte 1: {1'b1, row[9:7], time[4:1]}
//Byte 2: {1'b1, time[11:5]}
//Byte 3: {1'b10, time[17:12]} 
unsigned int CeleX5DataReader::getTimeStamp(unsigned char data[EVENT_SIZE])
{
	unsigned int low5bit = static_cast<unsigned int>(data[1] & 0x0F) << 1; //8'b0000 1111
	unsigned int mid7bit = static_cast<unsigned int>(data[2] & 0x7F);      //8'b0111 1111
	unsigned int high6bit = static_cast<unsigned int>(data[3] & 0x3F);     //8'b0011 1111
	return low5bit + (mid7bit << 5) + (high6bit << 12);
}

//-- CeleX4
//Byte 1: {1'b0, C[0], X[8:7], A[3:0]}
//Byte 2: {3'b0, A[8:4]}
//-- CeleX5
//Byte 1: {1'b0, col[10:7], A[2:0]}
//Byte 2: {3'b0, A[9:3]}
//Byte 3: {2'b00, A[11:10], mode[2:0], 1'b0}
unsigned int CeleX5DataReader::getADC(unsigned char data[EVENT_SIZE]) //ADC
{
	unsigned int low3bit = static_cast<unsigned int>(data[1] & 0x07);  //8'b0000 0111
	unsigned int mid7bit = static_cast<unsigned int>(data[2] & 0x7F);  //8'b0111 1111
	unsigned int high2bit = static_cast<unsigned int>(data[3] & 0x30); //8'b0011 0000, need >> 4
	return low3bit + (mid7bit << 3) + (high2bit << 6); // << (10 - 4)
}