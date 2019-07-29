#pragma once
#include <stdint.h>
#include <vector>
#include "CeleDriver.h"

typedef enum
{
	BUFFER_STATUS_EMPTY = 0,
	BUFFER_STATUS_FULL,
}BUFFER_STATUS;

<<<<<<< HEAD
#define  MAX_ELEMENT_BUFFER_SIZE   42976 //24576 / 24544 / 32752
=======
#define  MAX_ELEMENT_BUFFER_SIZE   43008 // 42976 / 24576 / 24544 / 32752
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea

class CElement
{
public:
	CElement();
	~CElement();

public:
	uint8_t *begin();
	uint8_t *end();
	void Save(uint8_t *buffer, uint16_t wLen);

private:
	uint8_t data[MAX_ELEMENT_BUFFER_SIZE];
	uint16_t wdataLen;
};

class CPackage
{
public:
	CPackage();
	~CPackage();

public:
	void Insert(uint8_t *buffer, uint16_t wLen);
	bool GetImage(std::vector<uint8_t> &Image);
	void End();
	void ClearData();
	int  Size();

<<<<<<< HEAD
	std::time_t           m_lTime_Stamp_End;
=======
	std::time_t                m_lTime_Stamp_End;
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
	std::vector<IMU_Raw_Data>  m_vecIMUData;

private:
	std::vector<CElement *>    element_list;
<<<<<<< HEAD
	BUFFER_STATUS         Status;
	size_t                Offset;
=======
	BUFFER_STATUS              Status;
	size_t                     Offset;
>>>>>>> 72687b79f3b7abd391838d295d21018c85d5c9ea
};

