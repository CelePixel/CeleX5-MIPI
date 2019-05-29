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

#include <opencv2/opencv.hpp>
#include "../include/celextypes.h"
#include "../include/dvslib/eventproc.h"

namespace dvs {

	class EventProcessing
	{
	public:
		EventProcessing() { }
		~EventProcessing() { }

		static int calculateDenoiseScore(const cv::Mat& currImg, int row, int col)
		{
			bool bHasPreImage = true;
			if (currImg.empty())
			{
				return 0;
			}
			if (m_matPreEventPicBuffer.empty())
			{
				m_matPreEventPicBuffer = cv::Mat::zeros(currImg.size(), CV_8UC1);
				bHasPreImage = false;
			}
			//calculate current image
			int count1 = 0;
			int count2 = 0;
			for (int i = row - 1; i < row + 2; ++i)
			{
				for (int j = col - 1; j < col + 2; ++j)
				{
					int index = i * PIXELS_PER_COL + j;
					if (index < 0 || index == row * PIXELS_PER_COL + col || index >= PIXELS_NUMBER)
						continue;
					if (currImg.ptr<uchar>(index / PIXELS_PER_COL)[index % PIXELS_PER_COL] > 0)
						++count1;
					else
						++count2;
				}
			}
			if (!bHasPreImage)
			{
				if (count1 >= count2)
					return 255;
				else
					return 0;
			}
			//calculate previous image 
			int index1 = (row - 1) * PIXELS_PER_COL + col;
			int index2 = row * PIXELS_PER_COL + col - 1;
			int index3 = row * PIXELS_PER_COL + col + 1;
			int index4 = (row + 1) * PIXELS_PER_COL + col;
			int aa[4] = { index1, index2, index3, index4 };
			int count3 = 0;
			int count4 = 0;
			for (int i = 0; i < 4; ++i)
			{
				if (aa[i] < 0 || aa[i] >= PIXELS_NUMBER)
					continue;
				if (m_matPreEventPicBuffer.ptr<uchar>(aa[i] / PIXELS_PER_COL)[aa[i] % PIXELS_PER_COL])
					++count3;
				else
					++count4;
			}
			if (count1 >= count2 || count3 >= count4)
				return 255;
			else
				return 0;
		}

		static void setPreviousImage(const cv::Mat& img)
		{
			m_matPreEventPicBuffer = img.clone();
		}

	private:
		static cv::Mat			m_matPreEventPicBuffer; //for denoise
	};
	cv::Mat	EventProcessing::m_matPreEventPicBuffer;
}

/********************************************************
*  @function :	SegmentationByMultislice
*  @brief    :	segmentate image by multislice image.
*  @input    :	multislicebyte : multislice image of uchar format
				ratio : the ratio of the motion area of whole image within the range [0,1]
*  @output   :	segimage : binary image after segment
*  @return   :	error code : 1 for true£¬0 for false
*  @author   :	xiongwc  2018/07/02 19:00
*  @History  :
*********************************************************/
int dvs::segmentationByMultislice(const cv::Mat &multislicebyte, double ratio, cv::Mat &segimage)
{
	if (multislicebyte.empty())
	{
		return 0;
	}
	segimage = cv::Mat::zeros(multislicebyte.size(), CV_8UC1);
	cv::threshold(multislicebyte, segimage, ratio * 255, 255, CV_THRESH_BINARY);
	return 1;
}

/********************************************************
*  @function :  DenoisingMaskByEventTime
*  @brief    :
*  @input    :  countEventImg : image of pixel assigned by event count;
				timelength : time length of frame
*  @output   :
*  @return   :
*  @author   :  xiongwc  2018/07/02 20:00
*  @History  :
*********************************************************/
int dvs::denoisingMaskByEventTime(const cv::Mat& countEventImg, double timelength, cv::Mat& denoiseMaskImg)
{
	if (countEventImg.empty())
	{
		return 0;
	}
	//cv::Mat kern = (cv::Mat_<uchar>(3, 3) << 0, 1, 0, 1, 1, 1, 0, 1, 0);//4-neighbor for convolution
	//cv::Mat convimg;
	//countEventImg.convertTo(convimg, CV_32FC1);
	//cv::filter2D(convimg, convimg, convimg.depth(), kern);

	//int timeslicecount = timelength / 800;//time step for density estimation, assigned by experience
	//int thresh = timeslicecount * 3;
	//cv::threshold(convimg, denoiseMaskImg, thresh, 255, CV_THRESH_BINARY);
	//denoiseMaskImg.convertTo(denoiseMaskImg, CV_8UC1);
	//return 1;

	cv::Mat kern = (cv::Mat_<uchar>(3, 3) << 1, 1, 1, 1, 1, 1, 1, 1, 1);
	cv::Mat convimg;
	cv::filter2D(countEventImg, convimg, countEventImg.depth(), kern);

	int timeslicecount = timelength / 60.0;//time step for density estimation, assigned by experience
	int thresh = timeslicecount * 5;
	cv::threshold(convimg, denoiseMaskImg, thresh, 255, CV_THRESH_BINARY);
	denoiseMaskImg.convertTo(denoiseMaskImg, CV_8UC1);
	return 1;
}

/********************************************************
*  @function :  DenoisingByNeighborhood
*  @brief    :	denoise the event image
*  @input    :  countEventImg : image of pixel assigned by event count;
*  @output   :	denoisedImg: image after denoising
*  @return   :
*  @author   :  renh  2018/07/10 10:00
*  @History  :
*********************************************************/
void dvs::denoisingByNeighborhood(const cv::Mat& countEventImg, cv::Mat& denoisedImg)
{
	const uchar* p;
	uchar* q;
	denoisedImg = cv::Mat::zeros(countEventImg.size(), CV_8UC1);
	for (int i = 0; i < countEventImg.rows; ++i)
	{
		p = countEventImg.ptr<uchar>(i);
		q = denoisedImg.ptr<uchar>(i);
		for (int j = 0; j < countEventImg.cols; ++j)
		{
			int score = p[j];
			if (score > 0)
			{
				score = EventProcessing::calculateDenoiseScore(countEventImg, i, j);
			}
			else
			{
				score = 0;
			}
			q[j] = score;
		}
	}
	EventProcessing::setPreviousImage(countEventImg);
}
