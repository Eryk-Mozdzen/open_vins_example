#include <cctype>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "SourceDataset.hpp"

SourceDataset::SourceDataset(Source::Listener *listener, const std::string path)
    : Source(listener), path{path} {

    {
        std::ifstream file(path + "/mav0/imu0/data.csv");
        std::string line;
        std::getline(file, line);
        std::getline(file, line);
        std::stringstream ss(line);
        std::string timestamp;
        std::getline(ss, timestamp, ',');
        reference = std::stol(timestamp);
    }

    {
        std::ifstream file(path + "/mav0/cam0/data.csv");
        std::string line;
        std::getline(file, line);
        std::getline(file, line);
        std::stringstream ss(line);
        std::string timestamp;
        std::getline(ss, timestamp, ',');
        reference = std::min(reference, std::stol(timestamp));
    }

    start = std::chrono::system_clock::now();

    threadIMU = std::thread(&SourceDataset::readIMU, this);
    threadCAM = std::thread(&SourceDataset::readCAM, this);
}

void SourceDataset::readIMU() {
    std::ifstream file(path + "/mav0/imu0/data.csv");
    std::string line;
    std::getline(file, line);

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
        sample.timestamp = std::stol(timestamp);
        sample.gyro[0] = std::stof(wx);
        sample.gyro[1] = std::stof(wy);
        sample.gyro[2] = std::stof(wz);
        sample.accel[0] = std::stof(ax);
        sample.accel[1] = std::stof(ay);
        sample.accel[2] = std::stof(az);

        std::this_thread::sleep_until(start +
                                      std::chrono::nanoseconds(sample.timestamp - reference));

        listener->available(sample);
    }
}

void SourceDataset::readCAM() {
    std::ifstream file(path + "/mav0/cam0/data.csv");
    std::string line;
    std::getline(file, line);

    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timestamp, filename;
        std::getline(ss, timestamp, ',');
        std::getline(ss, filename);

        filename.erase(filename.find_last_not_of(" \t\n\r\f\v") + 1);

        Source::CAM sample;
        sample.timestamp = std::stol(timestamp);
        cv::cvtColor(cv::imread(path + "/mav0/cam0/data/" + filename), sample.img0,
                     cv::COLOR_BGR2GRAY);
        // cv::cvtColor(cv::imread(path + "/mav0/cam1/data/" + filename), sample.img1,
        // cv::COLOR_BGR2GRAY);

        std::this_thread::sleep_until(start +
                                      std::chrono::nanoseconds(sample.timestamp - reference));

        listener->available(sample);
    }
}
