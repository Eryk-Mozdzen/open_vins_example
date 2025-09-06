#include <core/VioManager.h>
#include <state/State.h>

#include "Client.hpp"

#define DATASET "../datasets/dataset-corridor1_512_16"

void readIMU(std::shared_ptr<ov_msckf::VioManager> vio) {
    std::ifstream file(DATASET "/mav0/imu0/data.csv");
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

        ov_core::ImuData data;
        data.timestamp = std::stod(timestamp) * 1E-9;
        data.wm = Eigen::Vector3d(std::stod(wx), std::stod(wy), std::stod(wz));
        data.am = Eigen::Vector3d(std::stod(ax), std::stod(ay), std::stod(az));

        vio->feed_measurement_imu(data);

        time += std::chrono::milliseconds(5);
        std::this_thread::sleep_until(time);
    }
}

void readCAM(std::shared_ptr<ov_msckf::VioManager> vio) {
    std::ifstream file(DATASET "/mav0/cam0/data.csv");
    std::string line;
    std::getline(file, line);

    ov_core::CameraData data;
    data.sensor_ids = {
        0,
        1,
    };
    data.images = {
        cv::Mat(cv::Size(512, 512), CV_8UC1, cv::Scalar(0)),
        cv::Mat(cv::Size(512, 512), CV_8UC1, cv::Scalar(0)),
    };
    data.masks = {
        cv::imread("../mask0.png"),
        cv::imread("../mask1.png"),
    };

    auto time = std::chrono::system_clock::now();

    while(std::getline(file, line)) {
        std::stringstream ss(line);
        std::string timestamp, filename;
        std::getline(ss, timestamp, ',');
        std::getline(ss, filename);

        cv::Mat cam0 = cv::imread(DATASET "/mav0/cam0/data/" + filename);
        cv::Mat cam1 = cv::imread(DATASET "/mav0/cam1/data/" + filename);

        data.timestamp = std::stod(timestamp) * 1E-9;
        cv::cvtColor(cam0, data.images[0], cv::COLOR_BGR2GRAY);
        cv::cvtColor(cam1, data.images[1], cv::COLOR_BGR2GRAY);

        vio->feed_measurement_camera(data);

        time += std::chrono::milliseconds(50);
        std::this_thread::sleep_until(time);
    }
}

int main() {
    const auto parser = std::make_shared<ov_core::YamlParser>("../config.yaml");

    ov_msckf::VioManagerOptions params;
    params.print_and_load(parser);

    auto vio = std::make_shared<ov_msckf::VioManager>(params);

    std::thread thread1(readIMU, vio);
    std::thread thread2(readCAM, vio);

    Client client;
    client.write("{\"command\" : \"clear\"}");
    client.write("{\"command\" : \"config\", \"theme\" : \"dark\", \"camera\" : \"orbit\"}");
    client.write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [0.2, 0.2, 0.2]}, \
        \"material\" : {\"color\" : [128, 128, 128]}, \
        \"transform\" : {\"translation\" : [0, 0, 0]} \
    }");
    client.write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker/x\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [1, 0.05, 0.05]}, \
        \"material\" : {\"color\" : [255, 0, 0]}, \
        \"transform\" : {\"translation\" : [0.5, 0, 0]} \
    }");
    client.write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker/y\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [0.05, 1, 0.05]}, \
        \"material\" : {\"color\" : [0, 255, 0]}, \
        \"transform\" : {\"translation\" : [0, 0.5, 0]} \
    }");
    client.write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker/z\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [0.05, 0.05, 1]}, \
        \"material\" : {\"color\" : [0, 0, 255]}, \
        \"transform\" : {\"translation\" : [0, 0, 0.5]} \
    }");

    while(true) {
        double timestamp;
        cv::Mat window;

        vio->get_active_image(timestamp, window);

        if((window.size().width > 0) && (window.size().height > 0)) {
            cv::imshow("window", window);
            cv::waitKey(1);
        }

        const auto q = vio->get_state()->_imu->quat();
        const auto p = vio->get_state()->_imu->pos();

        std::stringstream ss;
        ss << "{";
        ss << "\"command\" : \"update\",";
        ss << "\"path\" : \"marker\",";
        ss << "\"transform\" : {";
        ss << "\"translation\" : [";
        ss << p[0] << ", ";
        ss << p[1] << ", ";
        ss << p[2] << "],";
        ss << "\"quaternion\" : [";
        ss << q[3] << ", ";
        ss << q[0] << ", ";
        ss << q[1] << ", ";
        ss << q[2] << "]";
        ss << "}";
        ss << "}";
        client.write(ss.str());

        std::stringstream ss2;
        ss2 << "{";
        ss2 << "\"command\" : \"camera\",";
        ss2 << "\"alpha\": 0.03,";
        ss2 << "\"position\" : [";
        ss2 << p[0] << ", ";
        ss2 << p[1] << ", ";
        ss2 << p[2] << "]";
        ss2 << "}";
        client.write(ss2.str());

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    thread1.join();
    thread2.join();
}
