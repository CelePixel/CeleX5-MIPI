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

#include "../include/celex5/celex5processeddata.h"

CeleX5ProcessedData::CeleX5ProcessedData()
	: m_iLoopNum(-1)
	, m_uiTemperature(0)
{
	m_pFullPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventBinaryPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventGrayPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventAccumulatedPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventCountPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventDenoisedBinaryPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pOpticalFlowPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pOpticalFlowSpeedPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pOpticalFlowDirectionPic = new unsigned char[CELEX5_PIXELS_NUMBER];
	m_pEventSuperimposedPic = new unsigned char[CELEX5_PIXELS_NUMBER];
}

CeleX5ProcessedData::~CeleX5ProcessedData()
{
	if (m_pFullPic) delete[] m_pFullPic;
	if (m_pEventBinaryPic) delete[] m_pEventBinaryPic;
	if (m_pEventGrayPic) delete[] m_pEventGrayPic;
	if (m_pEventAccumulatedPic) delete[] m_pEventAccumulatedPic;
	if (m_pEventCountPic) delete[] m_pEventCountPic;
	if (m_pEventDenoisedBinaryPic)delete[]m_pEventDenoisedBinaryPic;
	if (m_pOpticalFlowPic) delete[] m_pOpticalFlowPic;
	if (m_pOpticalFlowSpeedPic) delete[] m_pOpticalFlowSpeedPic;
	if (m_pOpticalFlowDirectionPic) delete[] m_pOpticalFlowDirectionPic;
	if (m_pEventSuperimposedPic)delete[] m_pEventSuperimposedPic;
}