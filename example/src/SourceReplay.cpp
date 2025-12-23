#include <cctype>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>

#include "SourceReplay.hpp"

SourceReplay::SourceReplay(Source::Listener &listener, const std::string path)
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

    threadIMU = std::thread(&SourceReplay::readIMU, this);
    threadCAM = std::thread(&SourceReplay::readCAM, this);
}

void SourceReplay::readIMU() {
    std::ifstream file(path + "/mav0/imu0/data.csv");
    std::string line;
    std::getline(file, line);

    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string t, wx, wy, wz, ax, ay, az;
        std::getline(ss, t, ',');
        std::getline(ss, wx, ',');
        std::getline(ss, wy, ',');
        std::getline(ss, wz, ',');
        std::getline(ss, ax, ',');
        std::getline(ss, ay, ',');
        std::getline(ss, az);

        const int64_t timestamp = std::stol(t);

        const float accel[3] = {
            std::stof(ax),
            std::stof(ay),
            std::stof(az),
        };

        const float gyro[3] = {
            std::stof(wx),
            std::stof(wy),
            std::stof(wz),
        };

        std::this_thread::sleep_until(start + std::chrono::nanoseconds(timestamp - reference));

        listener.handle(timestamp, accel, gyro);
    }
}

void SourceReplay::readCAM() {
    std::ifstream file(path + "/mav0/cam0/data.csv");
    std::string line;
    std::getline(file, line);

    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string t, filename;
        std::getline(ss, t, ',');
        std::getline(ss, filename);

        filename.erase(filename.find_last_not_of(" \t\n\r\f\v") + 1);

        const int64_t timestamp = std::stol(t);

        cv::Mat img;
        cv::cvtColor(cv::imread(path + "/mav0/cam0/data/" + filename), img, cv::COLOR_BGR2GRAY);

        std::this_thread::sleep_until(start + std::chrono::nanoseconds(timestamp - reference));

        listener.handle(timestamp, img);
    }
}
