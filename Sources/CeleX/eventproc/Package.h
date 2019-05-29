#pragma once
#include <stdint.h>
#include <vector>
#include <queue>

using namespace std;

typedef enum
{
	BUFFER_STATUS_EMPTY = 0,
	BUFFER_STATUS_FULL,
}BUFFER_STATUS;

#define  MAX_ELEMENT_BUFFER_SIZE   42976 //24576 / 24544 / 32752

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
	bool GetImage(vector<uint8_t> &Image);
	void End();
	void ClearData();

private:
	vector<CElement *>    element_list;
	BUFFER_STATUS         Status;
	size_t                Offset;
};


