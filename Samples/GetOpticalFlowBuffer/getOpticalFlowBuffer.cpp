/*
* Copyright (c) 2017-2018 CelePixel Technology Co. Ltd. All Rights Reserved
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
#include <celex5/celex5.h>

int main()
{
	CeleX5 *pCeleX = new CeleX5;
	if (NULL == pCeleX)
		return 0;
	pCeleX->openSensor(CeleX5::CeleX5_MIPI);
	pCeleX->setSensorFixedMode(CeleX5::Full_Optical_Flow_S_Mode);

	int imgSize = 1280 * 800;
	unsigned char* pOpticalFlowBuffer = new unsigned char[imgSize];
	while (true)
	{
		pCeleX->getOpticalFlowPicBuffer(pOpticalFlowBuffer); //optical-flow raw buffer
		cv::Mat matOpticalRaw(800, 1280, CV_8UC1, pOpticalFlowBuffer);
		//cv::Mat matOpticalRaw = pCeleX->getOpticalFlowPicMat();	//You can get the Mat form of optical flow pic directly.
		cv::imshow("Optical-Flow Buffer - Gray", matOpticalRaw);

		//optical-flow raw data - display color image
		cv::Mat matOpticalRawColor(800, 1280, CV_8UC3);
		uchar* pRaw = matOpticalRawColor.ptr<uchar>(0);
		int index = 0;
		for (int i = 0; i < matOpticalRawColor.rows; ++i)
		{
			cv::Vec3b *p = matOpticalRawColor.ptr<cv::Vec3b>(i);
			for (int j = 0; j < matOpticalRawColor.cols; ++j)
			{
				int value = matOpticalRaw.at<uchar>(i, j);
				//std::cout << value << std::endl;
				if (value == 0)
				{
					p[j][0] = 0;
					p[j][1] = 0;
					p[j][2] = 0;
				}
				else if (value < 50)	//blue
				{
					p[j][0] = 255;
					p[j][1] = 0;
					p[j][2] = 0;
				}
				else if (value < 100)
				{
					p[j][0] = 255;
					p[j][1] = 255;
					p[j][2] = 0;
				}
				else if (value < 150)	//green
				{
					p[j][0] = 0;
					p[j][1] = 255;
					p[j][2] = 0;
				}
				else if (value < 200)
				{
					p[j][0] = 0;
					p[j][1] = 255;
					p[j][2] = 255;
				}
				else	//red
				{
					p[j][0] = 0;
					p[j][1] = 0;
					p[j][2] = 255;
				}
			}
		}
		cv::imshow("Optical-Flow Buffer - Color", matOpticalRawColor);
		cvWaitKey(10);
	}
	return 1;
}