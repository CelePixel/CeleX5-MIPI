/*
* Copyright (c) 2017-2020  CelePixel Technology Co. Ltd.  All rights reserved.
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

#ifndef DATARECORDER_H
#define DATARECORDER_H

#include <vector>
#include "../include/celex5/celex5.h"

class DataRecorder
{
public:
	DataRecorder();
	~DataRecorder();

	/*
	*  @function: isRecording
	*  @brief   : check if it is recording
	*  @input   :
	*  @output  :
	*  @return  : true is recording, false is not recording
	*/
	bool isRecording()
	{
		return m_bRecording;
	}
	bool startRecording(std::string filePath);
	void stopRecording(CeleX5::BinFileAttributes* header);
	bool writeData(uint8_t* pData, uint32_t length);
	bool writeData(uint8_t* pData, uint32_t length, std::time_t time_stamp_end, std::vector<IMURawData> imuData);

private:
	int getLocalTimestamp();

private:
	bool           m_bRecording;
	int            m_iTimeStampStart;
	std::ofstream  m_ofstreamRecord;
	uint32_t       m_uiPackageCount;
};

#endif  // DATARECORDER_H

