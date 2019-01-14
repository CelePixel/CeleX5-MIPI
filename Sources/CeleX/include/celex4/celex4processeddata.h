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

#ifndef CELEX4_PROCESSED_DATA_H
#define CELEX4_PROCESSED_DATA_H

#include "celex4.h"
#include <opencv2/opencv.hpp>
class CeleX4ProcessedData
{
public:
	CeleX4ProcessedData();
	~CeleX4ProcessedData();

	inline emSensorMode getSensorMode() { return m_emSensorMode; }
	void setSensorMode(emSensorMode mode) { m_emSensorMode = mode; }
	//----------------save vector for a frame-------------
	inline void setEventDataVector(std::vector<EventData> eventData) { m_vectorEventData.swap(eventData); }
	inline std::vector<EventData> getEventDataVector() { return m_vectorEventData; }

	inline void setFrameDataVector(FrameData frameData) { m_frameData = frameData; }
	inline FrameData getFrameDataVector() { return m_frameData; }

	inline void setFixedNumEventDataVector(FrameData fixedeventData) { m_fixedEventData= fixedeventData; }
	inline FrameData getFixedNumEventDataVector() { return m_fixedEventData; }

	inline void setIMUDataVector(std::vector<IMUData> IMUData) { m_vectorIMUData.swap(IMUData); }
	inline std::vector<IMUData> getIMUDataVector() { return m_vectorIMUData; }

	inline unsigned char* getFullPicBuffer() { return m_pFullPicBuffer; }
	inline cv::Mat getFullPicMat() { return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pFullPicBuffer); }
	inline unsigned char* getEventPicBuffer(emEventPicMode mode)
	{
		switch (mode)
		{
		case EventBinaryPic:
			return m_pEventBinaryPic;
		case EventAccumulatedPic:
			return m_pEventAccumulatedPic;
		case EventGrayPic:
			return m_pEventGrayPic;
		case EventSuperimposedPic:
			return m_pEventSuperimposedPic;
		case EventDenoisedBinaryPic:
			return m_pEventDenoisedBinaryPic;
		case EventDenoisedGrayPic:
			return m_pEventDenoisedGrayPic;
		case EventCountPic:
			return m_pEventCountPic;
		//case EventDenoisedByTimeBinaryPic:
		//	return m_pEventDenoisedByTimeBinaryPic;
		//case EventDenoisedByTimeGrayPic:
		//	return m_pEventDenoisedByTimeGrayPic;
		default:
			break;
		}
		return NULL;
	}
	inline cv::Mat getEventPicMat(emEventPicMode mode)
	{
		switch (mode)
		{
		case EventBinaryPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventBinaryPic); 
		case EventAccumulatedPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventAccumulatedPic); 
		case EventGrayPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventGrayPic); 
		case EventSuperimposedPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventSuperimposedPic); 
		case EventDenoisedBinaryPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventDenoisedBinaryPic); 
		case EventDenoisedGrayPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventDenoisedGrayPic); 
		case EventCountPic:
			return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventCountPic);
		//case EventDenoisedByTimeBinaryPic:
		//	return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventDenoisedByTimeBinaryPic);
		//case EventDenoisedByTimeGrayPic:
		//	return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventDenoisedByTimeGrayPic);
		default:
			break;
		}
	}
	inline unsigned char* getOpticalFlowPicBuffer() { return m_pEventOpticalFlow; }
	inline unsigned char* getOpticalFlowDirectionPicBuffer() { return m_pEventOpticalFlowDirection; }
	inline unsigned char* getOpticalFlowSpeedPicBuffer() { return m_pEventOpticalFlowSpeed; }

	inline cv::Mat getOpticalFlowPicMat() { return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventOpticalFlow); }
	inline cv::Mat getOpticalFlowDirectionPicMat() { return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventOpticalFlowDirection); }
	inline cv::Mat getOpticalFlowSpeedPicMat() { return cv::Mat(cv::Size(768, 640), CV_8UC1, m_pEventOpticalFlowSpeed); }

private:
	std::vector<EventData> m_vectorEventData;	
	std::vector<IMUData> m_vectorIMUData;	
	FrameData			 m_frameData;	
	FrameData			 m_fixedEventData;
	unsigned char*    m_pFullPicBuffer;
	unsigned char*    m_pEventBinaryPic;
	unsigned char*    m_pEventAccumulatedPic;
	unsigned char*    m_pEventGrayPic;
	unsigned char*    m_pEventSuperimposedPic;
	unsigned char*    m_pEventDenoisedBinaryPic;
	unsigned char*    m_pEventDenoisedGrayPic;
	unsigned char*    m_pEventOpticalFlow;
	unsigned char*    m_pEventOpticalFlowDirection;
	unsigned char*    m_pEventOpticalFlowSpeed;
	unsigned char*	  m_pEventCountPic; 
	unsigned char*	  m_pEventDenoisedByTimeBinaryPic;
	unsigned char*	  m_pEventDenoisedByTimeGrayPic;

	int               m_nMeanIntensity;
	emSensorMode	  m_emSensorMode;
};

#endif // CELEX4_PROCESSED_DATA_H
