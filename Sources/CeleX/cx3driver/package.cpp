/*
* Copyright (c) 2017-2020 CelePixel Technology Co. Ltd. All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <string.h>
#include "package.h"

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

void CElement::save(uint8_t *buffer, uint16_t wLen)
{
	if (wLen > MAX_ELEMENT_BUFFER_SIZE)
		wLen = MAX_ELEMENT_BUFFER_SIZE;
	memcpy(data, buffer, wLen);
	wdataLen = wLen;
}

CPackage::CPackage()
{
	m_emStatus = BUFFER_STATUS_EMPTY;
	m_uiOffset = 0;
	m_lTimestampEnd = 0;
#ifdef NOT_USE_VECTOR
	m_pPackageBuffer = new uint8_t[MAX_ELEMENT_BUFFER_SIZE * 50];
#else
	for (int i = 0; i < 50; i++)
	{
		CElement *element = new CElement();
		if (element)
		{
			m_vecElementList.push_back(element);
		}
	}
#endif // _NOT_USE_VECTOR_
}

CPackage::~CPackage()
{
#ifdef NOT_USE_VECTOR
	if (m_pPackageBuffer != nullptr)
	{
		delete[] m_pPackageBuffer;
		m_pPackageBuffer = nullptr;
	}
#else
	while (!m_vecElementList.empty())
	{
		CElement *element = *m_vecElementList.begin();
		m_vecElementList.erase(m_vecElementList.begin());
		delete element;
	}
#endif
}

void CPackage::insert(uint8_t *buffer, uint16_t wLen)
{
#ifdef NOT_USE_VECTOR
	if (m_uiOffset < MAX_ELEMENT_BUFFER_SIZE * 50 - wLen)
	{
		memcpy(m_pPackageBuffer + m_uiOffset, buffer, wLen);
		m_uiOffset += wLen;
	}
	else
	{
		//printf("-------------- CPackage::insert wrong data --------------\n");
	}
#else
	if (m_uiOffset < m_vecElementList.size())
	{
		m_vecElementList[m_uiOffset]->save(buffer, wLen);
		m_uiOffset++;
	}
	else
	{
		//("-------------- CPackage::insert wrong data --------------\n");
	}
#endif
}

void CPackage::end(void)
{
	m_emStatus = BUFFER_STATUS_FULL;
}

void CPackage::clearData()
{
	m_emStatus = BUFFER_STATUS_EMPTY;
	m_uiOffset = 0;
}

int CPackage::size()
{
	return m_uiOffset;
}

bool CPackage::getImage(std::vector<uint8_t> &image)
{
	image.clear();
	if (m_emStatus == BUFFER_STATUS_FULL)
	{
		for (size_t i = 0; i < m_uiOffset; i++)
		{
			image.insert(image.end(), m_vecElementList[i]->begin(), m_vecElementList[i]->end());
		}
		m_uiOffset = 0;
		m_emStatus = BUFFER_STATUS_EMPTY;
		return true;
	}
	return false;
}

bool CPackage::getData(uint8_t* pData, uint32_t& length)
{
	if (nullptr == pData)
		return false;
	if (m_emStatus == BUFFER_STATUS_FULL)
	{	
#ifdef NOT_USE_VECTOR
		if (m_uiOffset > 1536001)
		{
			m_uiOffset = 0;
			m_emStatus = BUFFER_STATUS_EMPTY;
			//printf("----------------- wrong data size = %d\n", m_uiOffset);
			return false;
		}
		memcpy(pData, m_pPackageBuffer, m_uiOffset);
		length = m_uiOffset;
#else
		int offSize = 0;
		int dataSize = 0;
		for (size_t i = 0; i < m_uiOffset; i++)
		{
			dataSize = m_vecElementList[i]->end() - m_vecElementList[i]->begin();
			if (offSize + dataSize > 1536001)
			{
				m_uiOffset = 0;
				m_emStatus = BUFFER_STATUS_EMPTY;
				//printf("----------------- wrong data size = %d\n", offSize + dataSize);
				return false;
			}
			memcpy(pData + offSize, m_vecElementList[i]->begin(), dataSize);
			offSize += dataSize;	
		}
		length = offSize;
#endif
		//printf("------------------- length = %d\n", length);
		m_uiOffset = 0;
		m_emStatus = BUFFER_STATUS_EMPTY;
		return true;
	}
	return false;
}