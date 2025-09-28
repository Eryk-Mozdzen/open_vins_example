#ifndef SOURCE_SENSOR_HPP
#define SOURCE_SENSOR_HPP

#include <libcamera/libcamera.h>
#include <thread>

#include "Source.hpp"

class SourceSensor : public Source {
    std::thread threadIMU;
    std::thread threadCAM;
    std::shared_ptr<libcamera::Camera> camera;

    void readIMU();
    void readCAM();
    void readyCAM(libcamera::Request *request);

public:
    SourceSensor(Source::Listener *listener);
};

#endif
