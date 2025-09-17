#include <fstream>
#include <sstream>
#include <thread>

#include "SourceDataset.hpp"

SourceDataset::SourceDataset(Source::Listener *listener, const std::string path)
    : Source(listener),
      path{path},
      threadIMU(&SourceDataset::readIMU, this),
      threadCAM(&SourceDataset::readCAM, this) {
}

void SourceDataset::readIMU() {
    std::ifstream file(path + "/mav0/imu0/data.csv");
    std::string line;
    std::getline(file, line);

    auto time = std::chrono::system_clock::now();

    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timestamp, wx, wy, wz, ax, ay, az;
        std::getline(ss, timestamp, ',');
        std::getline(ss, wx, ',');
        std::getline(ss, wy, ',');
        std::getline(ss, wz, ',');
        std::getline(ss, ax, ',');
        std::getline(ss, ay, ',');
        std::getline(ss, az);

        Source::IMU sample;
        sample.timestamp = std::stoul(timestamp);
        sample.gyro[0] = std::stof(wx);
        sample.gyro[1] = std::stof(wy);
        sample.gyro[2] = std::stof(wz);
        sample.accel[0] = std::stof(ax);
        sample.accel[1] = std::stof(ay);
        sample.accel[2] = std::stof(az);

        listener->available(sample);

        time += std::chrono::milliseconds(5);
        std::this_thread::sleep_until(time);
    }
}

void SourceDataset::readCAM() {
    std::ifstream file(path + "/mav0/cam0/data.csv");
    std::string line;
    std::getline(file, line);

    auto time = std::chrono::system_clock::now();

    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timestamp, filename;
        std::getline(ss, timestamp, ',');
        std::getline(ss, filename);

        Source::CAM sample;
        sample.timestamp = std::stoul(timestamp);
        sample.img0 = cv::imread(path + "/mav0/cam0/data/" + filename);
        sample.img1 = cv::imread(path + "/mav0/cam1/data/" + filename);

        listener->available(sample);

        time += std::chrono::milliseconds(50);
        std::this_thread::sleep_until(time);
    }
}
