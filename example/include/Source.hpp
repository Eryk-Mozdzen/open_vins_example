#ifndef SOURCE_HPP
#define SOURCE_HPP

#include <opencv2/opencv.hpp>

class Source {
public:
    class Listener {
    public:
        virtual void handle(const int64_t timestamp, const float accel[3], const float gyro[3]) = 0;
        virtual void handle(const int64_t timestamp, const cv::Mat &img) = 0;
    };

protected:
    Listener &listener;

    Source(Listener &listener);
};

#endif
