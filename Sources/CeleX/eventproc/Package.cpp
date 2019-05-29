#include <string.h>
#include <iostream>
#include "Package.h"

using namespace std;

CElement::CElement()
{
	wdataLen = 0;
}

CElement::~CElement()
{

}

uint8_t *CElement::begin()
{
	return data;
}
uint8_t *CElement::end()
{
	return data + wdataLen;
}

void CElement::Save(uint8_t *buffer, uint16_t wLen)
{
	if (wLen > MAX_ELEMENT_BUFFER_SIZE)
		wLen = MAX_ELEMENT_BUFFER_SIZE;
	memcpy(data, buffer, wLen);
	wdataLen = wLen;
}

CPackage::CPackage()
{
	Status = BUFFER_STATUS_EMPTY;
	Offset = 0;
	for(int i = 0; i < 50; i++)
    {
        CElement *element = new CElement();
		if (element)
		{
			element_list.push_back(element);
		}
    }
}

CPackage::~CPackage()
{
	while (!element_list.empty())
	{
		CElement *element = *element_list.begin();
		element_list.erase(element_list.begin());
		delete element;
	}
}

void CPackage::Insert(uint8_t *buffer, uint16_t wLen)
{
	if (Offset < element_list.size())
	{
		element_list[Offset]->Save(buffer, wLen);
		Offset++;
	}
	else
	{
		CElement *element = new CElement();
		if (element)
		{
			element->Save(buffer, wLen);
			element_list.push_back(element);
			Offset++;
		}
	}
}

void CPackage::End(void)
{
	Status = BUFFER_STATUS_FULL;
}

void CPackage::ClearData()
{
	Status = BUFFER_STATUS_EMPTY;
	Offset = 0;
	element_list.clear();
}

bool CPackage::GetImage(vector<uint8_t> &Image)
{
	Image.clear();
	if (Status == BUFFER_STATUS_FULL)
	{
		for (size_t i = 0; i < Offset; i++)
		{
			Image.insert(Image.end(),element_list[i]->begin(), element_list[i]->end());
		}
		Offset = 0;
		Status = BUFFER_STATUS_EMPTY;
		return true;
	}
	return false;
}