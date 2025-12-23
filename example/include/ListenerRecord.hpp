#ifndef LISTENER_RECORD_HPP
#define LISTENER_RECORD_HPP

#include <fstream>
#include <string>

#include "Source.hpp"

class ListenerRecord : public Source::Listener {
    const std::string path;
    std::ofstream csvIMU;
    std::ofstream csvCAM;

    void handle(const int64_t timestamp, const float accel[3], const float gyro[3]) override;
    void handle(const int64_t timestamp, const cv::Mat &img) override;

public:
    ListenerRecord(const std::string path);
};

#endif
