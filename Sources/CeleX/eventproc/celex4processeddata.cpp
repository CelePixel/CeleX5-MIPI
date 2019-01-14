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

#include "../include/celex4/celex4.h"
#include "../include/celex4/celex4processeddata.h"

CeleX4ProcessedData::CeleX4ProcessedData()
	: m_pFullPicBuffer(NULL)
	, m_pEventBinaryPic(NULL)
	, m_pEventAccumulatedPic(NULL)
	, m_pEventGrayPic(NULL)
	, m_pEventSuperimposedPic(NULL)
	, m_pEventDenoisedBinaryPic(NULL)
	, m_pEventDenoisedGrayPic(NULL)
	, m_pEventCountPic(NULL)
	, m_pEventOpticalFlow(NULL)
	, m_pEventOpticalFlowDirection(NULL)
	, m_pEventOpticalFlowSpeed(NULL)
	, m_pEventDenoisedByTimeBinaryPic(NULL)
	, m_pEventDenoisedByTimeGrayPic(NULL)
	, m_nMeanIntensity(0)
{
	m_pFullPicBuffer = new unsigned char[PIXELS_NUMBER];
	m_pEventBinaryPic = new unsigned char[PIXELS_NUMBER];
	m_pEventAccumulatedPic = new unsigned char[PIXELS_NUMBER];
	m_pEventGrayPic = new unsigned char[PIXELS_NUMBER];
	m_pEventSuperimposedPic = new unsigned char[PIXELS_NUMBER];
	m_pEventDenoisedBinaryPic = new unsigned char[PIXELS_NUMBER];
	m_pEventDenoisedGrayPic = new unsigned char[PIXELS_NUMBER];
	m_pEventCountPic = new unsigned char[PIXELS_NUMBER];
	m_pEventOpticalFlow = new unsigned char[PIXELS_NUMBER];
	m_pEventOpticalFlowDirection = new unsigned char[PIXELS_NUMBER];
	m_pEventOpticalFlowSpeed = new unsigned char[PIXELS_NUMBER];
	m_pEventDenoisedByTimeBinaryPic = new unsigned char[PIXELS_NUMBER];
	m_pEventDenoisedByTimeGrayPic = new unsigned char[PIXELS_NUMBER];
}

CeleX4ProcessedData::~CeleX4ProcessedData()
{
}
