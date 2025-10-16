#ifndef SOURCE_HPP
#define SOURCE_HPP

#include <opencv2/opencv.hpp>

class Source {
public:
    struct IMU {
        int64_t timestamp;
        float accel[3];
        float gyro[3];
    };

    struct CAM {
        int64_t timestamp;
        cv::Mat img0;
        cv::Mat img1;
    };

    class Listener {
    public:
        virtual void available(const IMU &sample) = 0;
        virtual void available(const CAM &sample) = 0;
    };

protected:
    Listener *listener;

    Source(Listener *listener);
};

#endif
