#include "celex5_ros.h"

namespace celex_ros {

CelexRos::CelexRos() {}

int CelexRos::initDevice() {}

void CelexRos::grabEventData(
    CeleX5 *celex,
    celex5_msgs::eventVector &msg) {
  if (celex->getSensorFixedMode() == CeleX5::Event_Off_Pixel_Timestamp_Mode) {
    std::vector<EventData> vecEvent;

    celex->getEventDataVector(vecEvent);

    int dataSize = vecEvent.size();
    msg.vectorIndex = 0;
    msg.height = MAT_ROWS;
    msg.width = MAT_COLS;
    msg.vectorLength = dataSize;

    cv::Mat mat = cv::Mat::zeros(cv::Size(MAT_COLS, MAT_ROWS), CV_8UC1);
    for (int i = 0; i < dataSize; i++) {
      mat.at<uchar>(MAT_ROWS - vecEvent[i].row - 1,
                    MAT_COLS - vecEvent[i].col - 1) = 255;
      event_.x = vecEvent[i].row;
      event_.y = vecEvent[i].col;
      event_.brightness = 255;
	  event_.timestamp = vecEvent[i].t_off_pixel;
      msg.events.push_back(event_);
    }
  } else {
    msg.vectorLength = 0;
    std::cout << "This mode has no event data. " << std::endl;
  }
}

CelexRos::~CelexRos() {}
}
