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

#ifndef CELEX5DATAREADER_H
#define CELEX5DATAREADER_H

#include "../include/celextypes.h"

class CeleX5DataReader
{
public:
	CeleX5DataReader();
	~CeleX5DataReader();

	//check event type
	bool isRowEvent(unsigned char data[EVENT_SIZE]);
	bool isRowEventEx(unsigned char data[EVENT_SIZE]);
	bool isColumnEvent(unsigned char data[EVENT_SIZE]);
	bool isSpecialEvent(unsigned char data[EVENT_SIZE]);

	void parseRowEvent(unsigned char data[EVENT_SIZE]);
	void parseColumnEvent(unsigned char data[EVENT_SIZE]);

	void parseRowEventEx(unsigned char data[EVENT_SIZE]);

	bool parseFPGAEvent(unsigned char data[EVENT_SIZE], unsigned int* col1, unsigned int* col2); //16bits for no adc event
	bool parseFPGAEventEx(unsigned char data[EVENT_SIZE], unsigned int* col1, unsigned int* adc1, unsigned int* col2, unsigned int* adc2); //24bits for adc event

	inline unsigned int row() { return m_uiRow; }
	inline unsigned int lastRow() { return m_uiLastRow; }
	inline unsigned int column() { return m_uiColumn; }
	inline unsigned int adc() { return m_uiADC; }
	inline unsigned int getTFromFPGA() { return m_uiTFromFPGA; }
	inline unsigned int getLastTFromFPGA() { return m_uiLastTFromFPGA; }
	inline unsigned int getEventType() { return m_uiEventType; }
	inline unsigned int getSensorMode() { return m_uiSensorMode; }

private:
	unsigned int getColumn(unsigned char data[EVENT_SIZE]);
	unsigned int getRow(unsigned char data[EVENT_SIZE]);
	unsigned int getTimeStamp(unsigned char data[EVENT_SIZE]);
	unsigned int getADC(unsigned char data[EVENT_SIZE]);

private:
	unsigned int  m_uiRow;
	unsigned int  m_uiLastRow;
	unsigned int  m_uiColumn;
	unsigned int  m_uiADC;
	unsigned int  m_uiTFromFPGA;
	unsigned int  m_uiLastTFromFPGA;
	unsigned int  m_uiEventType;
	unsigned int  m_uiSensorMode;
	unsigned char m_lastData[4];
};

#endif // CELEX5DATAREADER_H
