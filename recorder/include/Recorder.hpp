#ifndef RECORDER_HPP
#define RECORDER_HPP

#include <fstream>
#include <string>

#include "Source.hpp"

class Recorder : public Source::Listener {
    const std::string path;
    std::ofstream csvIMU;
    std::ofstream csvCAM;

    void available(const Source::IMU &sample) override;
    void available(const Source::CAM &sample) override;

public:
    Recorder(const std::string path);
};

#endif
