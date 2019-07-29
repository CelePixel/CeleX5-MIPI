#include "celex5_ros.h"

static const std::string OPENCV_WINDOW = "Image window";

namespace celex_ros {
class CelexRosNode {
public:
  // private ROS node handle
  ros::NodeHandle node_;

  // custom celex5 message type
  celex5_msgs::eventVector event_vector_;

  ros::Publisher data_pub_, image_pub_;
  ros::Subscriber data_sub_;

  // parameters
  std::string celex_mode_;
  int threshold_, clock_rate_;

  CelexRos celexRos_;
  CeleX5 *celex_;

  CelexRosNode() : node_("~") {

    image_pub_ = node_.advertise<sensor_msgs::Image>("/celex_image",1);

    data_pub_ = node_.advertise<celex5_msgs::eventVector>("celex5_event", 1);
    data_sub_ =
        node_.subscribe("celex5_event", 1, &CelexRosNode::celexDataCallback, this);

    // grab the parameters
    node_.param<std::string>("celex_mode", celex_mode_,
                             "Event_Off_Pixel_Timestamp_Mode");

    node_.param<int>("threshold", threshold_, 170);   // 0-1024
    node_.param<int>("clock_rate", clock_rate_, 100); // 0-100

    cv::namedWindow(OPENCV_WINDOW);
  }

  ~CelexRosNode() { cv::destroyWindow(OPENCV_WINDOW); }
  // subscribe callback function
  void celexDataCallback(const celex5_msgs::eventVector &msg);

  bool grabAndSendData();
  void setCeleX5(CeleX5 *pcelex);
  bool spin();
};

void CelexRosNode::celexDataCallback(const celex5_msgs::eventVector &msg) {
  ROS_INFO("I heard celex5 data size: [%d]", msg.vectorLength);
  if (msg.vectorLength > 0) {
    cv::Mat mat = cv::Mat::zeros(cv::Size(MAT_COLS, MAT_ROWS), CV_8UC1);
    for (int i = 0; i < msg.vectorLength; i++) {
      mat.at<uchar>(MAT_ROWS - msg.events[i].x - 1,
                    MAT_COLS - msg.events[i].y - 1) = msg.events[i].brightness;
    }
    cv::imshow(OPENCV_WINDOW, mat);
    cv::waitKey(1);
  }
}

bool CelexRosNode::grabAndSendData() {
  celexRos_.grabEventData(celex_, event_vector_);
  data_pub_.publish(event_vector_);
  event_vector_.events.clear();

  // get sensor image and publish it
  cv::Mat image = celex_->getEventPicMat(CeleX5::EventBinaryPic);
  sensor_msgs::ImagePtr msg =
      cv_bridge::CvImage(std_msgs::Header(), "mono8", image).toImageMsg();
  image_pub_.publish(msg);
}

void CelexRosNode::setCeleX5(CeleX5 *pcelex) {
  celex_ = pcelex;

  celex_->setThreshold(threshold_);

  CeleX5::CeleX5Mode mode;
  if (celex_mode_ == "Event_Off_Pixel_Timestamp_Mode")
    mode = CeleX5::Event_Off_Pixel_Timestamp_Mode;
  else if (celex_mode_ == "Full_Picture_Mode")
    mode = CeleX5::Full_Picture_Mode;

  celex_->setSensorFixedMode(mode);
}

bool CelexRosNode::spin() {
  ros::Rate loop_rate(30);

  while (node_.ok()) {
    grabAndSendData();
    ros::spinOnce();
    loop_rate.sleep();
  }
  return true;
}
}

int main(int argc, char **argv) {
  ros::init(argc, argv, "celex_monocular");

  CeleX5 *pCelex_;
  pCelex_ = new CeleX5;
  if (NULL == pCelex_)
    return 0;
  pCelex_->openSensor(CeleX5::CeleX5_MIPI);

  celex_ros::CelexRosNode cr;
  cr.setCeleX5(pCelex_);
  cr.spin();
  return EXIT_SUCCESS;
}
