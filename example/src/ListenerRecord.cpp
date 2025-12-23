#include <filesystem>
#include <fstream>
#include <string>

#include "ListenerRecord.hpp"

ListenerRecord::ListenerRecord(const std::string path) : path{path} {
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

void ListenerRecord::handle(const int64_t timestamp, const float accel[3], const float gyro[3]) {
    csvIMU << timestamp << "," << gyro[0] << "," << gyro[1] << "," << gyro[2] << "," << accel[0]
           << "," << accel[1] << "," << accel[2] << std::endl;
}

void ListenerRecord::handle(const int64_t timestamp, const cv::Mat &img) {
    const std::string name = std::to_string(timestamp) + ".png";

    cv::imwrite(path + "/mav0/cam0/data/" + name, img);

    csvCAM << timestamp << "," << name << std::endl;
}
