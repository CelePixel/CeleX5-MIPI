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

#define FPN_PATH    "../Samples/config/FPN_3.txt"

int main()
{
	CeleX5 *pCeleX = new CeleX5;

	if (pCeleX == NULL)
		return 0;

	pCeleX->openSensor(CeleX5::CeleX5_MIPI);
	pCeleX->setFpnFile(FPN_PATH);

	CeleX5::CeleX5Mode sensorMode = CeleX5::Event_Address_Only_Mode;
	pCeleX->setSensorFixedMode(sensorMode);
	pCeleX->setClockRate(100);

	while (true)
	{
		if (sensorMode == CeleX5::Full_Picture_Mode)
		{
			if (!pCeleX->getFullPicMat().empty())
			{
				cv::Mat fullPicMat = pCeleX->getFullPicMat();
				cv::imshow("FullPic", fullPicMat);
				cv::waitKey(10);
			}
		}
		else if (sensorMode == CeleX5::Event_Address_Only_Mode)
		{
			if (!pCeleX->getEventPicMat(CeleX5::EventBinaryPic).empty())
			{
				cv::Mat eventPicMat = pCeleX->getEventPicMat(CeleX5::EventBinaryPic);
				cv::imshow("Event-EventBinaryPic", eventPicMat);
			}
			cv::waitKey(10);
		}
	}
	return 1;

}
