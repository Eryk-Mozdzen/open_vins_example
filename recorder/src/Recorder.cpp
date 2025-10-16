#include <filesystem>
#include <fstream>
#include <string>

#include "Recorder.hpp"

Recorder::Recorder(const std::string path) : path{path} {
    std::filesystem::create_directories(path + "/mav0/imu0");
    std::filesystem::create_directories(path + "/mav0/cam0");
    std::filesystem::create_directories(path + "/mav0/cam0/data");

    csvIMU.open(path + "/mav0/imu0/data.csv");
    csvCAM.open(path + "/mav0/cam0/data.csv");

    csvIMU << "#timestamp [ns],w_RS_S_x [rad s^-1],w_RS_S_y [rad s^-1],w_RS_S_z [rad "
              "s^-1],a_RS_S_x [m s^-2],a_RS_S_y [m s^-2],a_RS_S_z [m s^-2]"
           << std::endl;

    csvCAM << "#timestamp [ns],filename" << std::endl;
}

void Recorder::available(const Source::IMU &sample) {
    csvIMU << sample.timestamp << "," << sample.gyro[0] << "," << sample.gyro[1] << ","
           << sample.gyro[2] << "," << sample.accel[0] << "," << sample.accel[1] << ","
           << sample.accel[2] << std::endl;
}

void Recorder::available(const Source::CAM &sample) {
    const std::string name = std::to_string(sample.timestamp) + ".png";

    cv::imwrite(path + "/mav0/cam0/data/" + name, sample.img0);

    csvCAM << sample.timestamp << "," << name << std::endl;
}
