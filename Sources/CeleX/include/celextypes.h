/*
* Copyright (c) 2017-2020 CelePixel Technology Co. Ltd. All Rights Reserved
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

#ifndef CELEXTYPES_H
#define CELEXTYPES_H

#include <stdint.h>
#include <ctime>

#define SLIDER_DELAY   100
#define MAX_LOG_LINES  100

#define MAX_PAGE_COUNT 100000

#define PIPEOUT_TIMER  10
#define EVENT_SIZE     4
#define PIXELS_PER_COL 768
#define PIXELS_PER_ROW 640
#define PIXELS_NUMBER  491520

#define CELEX5_COL            1280
#define CELEX5_ROW            800
#define  CELEX5_PIXELS_NUMBER 1024000

#define MIRROR_VERTICAL     1
#define MIRROR_HORIZONTAL   1

#define FILE_CELEX5_CFG              "CeleX5_Commands_MIPI.xml"
#define FILE_CELEX5_CFG_MIPI         "cfg_mp"
#define FILE_CELEX5_CFG_MIPI_WRIE    "cfg_mp_wire"

#define SEQUENCE_LAYOUT_WIDTH 3 //7
#define SLIDER_LAYOUT_WIDTH   1 //4
#define DIALOG_LAYOUT_WIDTH   2

#define FPN_CALCULATION_TIMES 5

#define BIAS_RAMPP_H                16
#define BIAS_RAMPP_L                17
#define BIAS_BRT_I_H                22
#define BIAS_BRT_I_L                23
#define BIAS_ADVH_I_H               26
#define BIAS_ADVH_I_L               27
#define BIAS_ADVL_I_H               30
#define BIAS_ADVL_I_L               31
#define BIAS_ADVCH_I_H              34
#define BIAS_ADVCH_I_L              35
#define BIAS_ADVCL_I_H              38
#define BIAS_ADVCL_I_L              39

#define BIAS_VCM_I_H                42
#define BIAS_VCM_I_L                43

#define ROW_ENABLE                  44
#define COL_GAIN                    45

#define SENSOR_MODE_1               53
#define SENSOR_MODE_2               54
#define SENSOR_MODE_3               55

#define EVENT_DURATION_H            58
#define EVENT_DURATION_L            57

#define PICTURE_NUMBER_1            59
#define PICTURE_NUMBER_2            60
#define PICTURE_NUMBER_3            61
#define PICTURE_NUMBER_4            62
#define PICTURE_NUMBER_5            63

#define SENSOR_MODE_SELECT          64

#define EVENT_PACKET_SELECT         73

#define MIPI_ROW_NUM_EVENT_H        79
#define MIPI_ROW_NUM_EVENT_L        80

#define MIPI_HD_GAP_FULLFRAME_H     82
#define MIPI_HD_GAP_FULLFRAME_L     83

#define MIPI_HD_GAP_EVENT_H         84
#define MIPI_HD_GAP_EVENT_L         85

#define MIPI_GAP_EOF_SOF_H          86
#define MIPI_GAP_EOF_SOF_L          87

#define SOFT_RESET                  90
#define SOFT_TRIGGER                93
#define PADDR_EN                    94

#define MIPI_PLL_DIV_I              113
#define MIPI_PLL_DIV_N_H            115
#define MIPI_PLL_DIV_N_L            114

#define MIPI_NPOWD_PLL              139
#define MIPI_NPOWD_BGR              140
#define MIPI_NRSET_PLL              141
#define MIPI_NPOWD_PHY              142
#define MIPI_NRSET_PHY              143
#define MIPI_NDIS_PHY               144

#define PLL_PD_B                    150
#define PLL_FOUT_DIV1               151
#define PLL_FOUT_DIV2               152
#define PLL_DIV_N                   159
#define PLL_DIV_L                   160
#define PLL_FOUT3_POST_DIV          169

#define FLICKER_DETECT_EN           183

#define AUTOISP_PROFILE_ADDR        220
#define AUTOISP_BRT_EN              221
#define AUTOISP_TEM_EN              222
#define AUTOISP_TRIGGER             223

#define AUTOISP_BRT_VALUE_H         233
#define AUTOISP_BRT_VALUE_L         232

#define AUTOISP_BRT_THRES1_H        235
#define AUTOISP_BRT_THRES1_L        234

#define AUTOISP_BRT_THRES2_H        237
#define AUTOISP_BRT_THRES2_L        236

#define AUTOISP_BRT_THRES3_H        239
#define AUTOISP_BRT_THRES3_L        238

#define VIRTUAL_USB_ADDR            254

typedef struct EventData
{
	uint16_t    col;
	uint16_t    row;
	uint16_t    adc; //Event_Off_Pixel_Timestamp_Mode: adc is 0; Event Intensity Mode: adc is "Intensity"; Event_In_Pixel_Timestamp_Mode: adc is "Optical-flow T"
	int16_t     polarity; //-1: intensity weakened; 1: intensity is increased; 0 intensity unchanged
	uint32_t    tInPixelIncreasing;
	uint32_t    tOffPixelIncreasing; //it won't be reset, it's a monotonically increasing value
} EventData;

typedef enum EventShowType
{
	EventShowByTime = 0,
	EventShowByCount = 1,
	EventShowByRowCycle = 2,
} EventShowType;

typedef enum PlaybackState {
	NoBinPlaying = 0,
	Playing,
	BinReadFinished,
	PlayFinished,
	Replay
} PlaybackState;

typedef struct IMUData {
	double			xGYROS;
	double			yGYROS;
	double			zGYROS;
	double			xACC;
	double			yACC;
	double			zACC;
	double			xMAG;
	double			yMAG;
	double			zMAG;
	double			xTEMP;
	uint64_t        frameNo;
	std::time_t     timestamp;
} IMUData;

typedef struct IMURawData
{
	uint8_t       imuData[20];
	std::time_t   timestamp;
} IMURawData;

#endif // CELEXTYPES_H
