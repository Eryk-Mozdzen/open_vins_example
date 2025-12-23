#ifndef SOURCE_HARDWARE_HPP
#define SOURCE_HARDWARE_HPP

#include <libcamera/libcamera.h>
#include <thread>

#include "Source.hpp"

class SourceHardware : public Source {
    std::thread threadIMU;
    std::thread threadCAM;
    std::shared_ptr<libcamera::Camera> camera;

    void readIMU();
    void readCAM();
    void readyCAM(libcamera::Request *request);

public:
    SourceHardware(Source::Listener *listener);
};

#endif
