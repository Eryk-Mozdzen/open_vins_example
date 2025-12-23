#ifndef LISTENER_OPENVINS_HPP
#define LISTENER_OPENVINS_HPP

#include <core/VioManager.h>

#include "Source.hpp"

class ListenerOpenVINS : public Source::Listener {
    std::shared_ptr<ov_msckf::VioManager> vio;

    void handle(const int64_t timestamp, const float accel[3], const float gyro[3]) override;
    void handle(const int64_t timestamp, const cv::Mat &img) override;

public:
    ListenerOpenVINS(const std::string configPath);

    cv::Mat getImage() const;
    Eigen::Vector4d getAttitude() const;
    Eigen::Vector3d getPosition() const;
    std::vector<Eigen::Vector3d> getFeatures() const;
};

#endif
