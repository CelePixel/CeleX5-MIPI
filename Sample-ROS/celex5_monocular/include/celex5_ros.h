#include <ros/ros.h>
#include <celex5/celex5.h>
#include <celex5/celex5datamanager.h>
#include <celextypes.h>
#include <sensor_msgs/Image.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <celex5_msgs/event.h>
#include <celex5_msgs/eventVector.h>
#include <cv_bridge/cv_bridge.h>

#define MAT_ROWS 800
#define MAT_COLS 1280
namespace celex_ros
{
    class CelexRos{
        public:

        CelexRos();
        ~CelexRos();

        int initDevice();
        void setSensorParams();
        void grabEventData(CeleX5 *celex, celex5_msgs::eventVector& msg);

        private:
        celex5_msgs::event event_;
    };
}