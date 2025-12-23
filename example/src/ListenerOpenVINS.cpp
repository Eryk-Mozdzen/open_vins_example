#include <core/VioManager.h>
#include <state/State.h>

#include "ListenerOpenVINS.hpp"

ListenerOpenVINS::ListenerOpenVINS(const std::string configPath) {
    const auto parser = std::make_shared<ov_core::YamlParser>(configPath);

    ov_msckf::VioManagerOptions params;
    params.print_and_load(parser);

    vio = std::make_shared<ov_msckf::VioManager>(params);
}

void ListenerOpenVINS::handle(const int64_t timestamp, const float accel[3], const float gyro[3]) {
    // printf("IMU %10lu ms %+7.2f %+7.2f %+7.2f %+7.2f %+7.2f %+7.2f\n", timestamp / 1000000LU,
    //        gyro[0], gyro[1], gyro[2], accel[0], accel[1], accel[2]);

    ov_core::ImuData data;
    data.timestamp = timestamp * 1E-9;
    data.wm = Eigen::Vector3d(gyro[0], gyro[1], gyro[2]);
    data.am = Eigen::Vector3d(accel[0], accel[1], accel[2]);

    vio->feed_measurement_imu(data);
}

void ListenerOpenVINS::handle(const int64_t timestamp, const cv::Mat &img) {
    // printf("CAM %10lu ms\n", timestamp / 1000000LU);

    ov_core::CameraData data;
    data.timestamp = timestamp * 1E-9;
    data.sensor_ids = {0};
    data.masks = {cv::Mat(img.size(), CV_8UC1, cv::Scalar(0))};
    data.images = {img};

    vio->feed_measurement_camera(data);
}

cv::Mat ListenerOpenVINS::getImage() const {
    double timestamp;
    cv::Mat image;

    vio->get_active_image(timestamp, image);

    return image;
}

Eigen::Vector4d ListenerOpenVINS::getAttitude() const {
    return vio->get_state()->_imu->quat();
}

Eigen::Vector3d ListenerOpenVINS::getPosition() const {
    return vio->get_state()->_imu->pos();
}

std::vector<Eigen::Vector3d> ListenerOpenVINS::getFeatures() const {
    return vio->get_features_SLAM();
}
