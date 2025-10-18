#include <core/VioManager.h>
#include <state/State.h>

#include "Source.hpp"
#include "SourceDataset.hpp"
#include "SourceHardware.hpp"
#include "Visualization.hpp"

class Listener : public Source::Listener {
    std::shared_ptr<ov_msckf::VioManager> vio;

    void available(const Source::IMU &sample) override {
        ov_core::ImuData data;
        data.timestamp = sample.timestamp * 1E-9;
        data.wm = Eigen::Vector3d(sample.gyro[0], sample.gyro[1], sample.gyro[2]);
        data.am = Eigen::Vector3d(sample.accel[0], sample.accel[1], sample.accel[2]);

        // printf("IMU %10lu ms %+7.2f %+7.2f %+7.2f %+7.2f %+7.2f %+7.2f\n",
        //        sample.timestamp / 1000000LU, sample.gyro[0], sample.gyro[1], sample.gyro[2],
        //        sample.accel[0], sample.accel[1], sample.accel[2]);

        vio->feed_measurement_imu(data);
    }

    void available(const Source::CAM &sample) override {
        ov_core::CameraData data;
        data.timestamp = sample.timestamp * 1E-9;
        data.sensor_ids = {0};
        data.masks = {cv::Mat(sample.img0.size(), CV_8UC1, cv::Scalar(0))};
        data.images = {sample.img0};

        // printf("CAM %10lu ms\n", sample.timestamp / 1000000LU);

        vio->feed_measurement_camera(data);
    }

public:
    Listener(std::shared_ptr<ov_msckf::VioManager> vio) : vio{vio} {
    }
};

int main() {
    // const auto parser = std::make_shared<ov_core::YamlParser>("../config/tum/config.yaml");
    // const auto parser = std::make_shared<ov_core::YamlParser>("../config/euroc/config.yaml");
    const auto parser = std::make_shared<ov_core::YamlParser>("../config/custom/config.yaml");

    ov_msckf::VioManagerOptions params;
    params.print_and_load(parser);

    auto vio = std::make_shared<ov_msckf::VioManager>(params);

    Listener listener(vio);

    // SourceDataset source(&listener, "../datasets/dataset-corridor1_512_16");
    // SourceDataset source(&listener, "../datasets/MH_01_easy");
    SourceDataset source(&listener, "../datasets/custom");
    // SourceHardware source(&listener);

    Visualization visualization;

    int points = 0;

    while(true) {
        /*double timestamp;
        cv::Mat window;

        vio->get_active_image(timestamp, window);

        if(!window.empty()) {
            cv::imshow("window", window);
            cv::waitKey(1);
        }*/

        const auto q = vio->get_state()->_imu->quat();
        const auto p = vio->get_state()->_imu->pos();

        visualization.update(q.data(), p.data());

        // const auto features = vio->get_features_SLAM();
        const auto features = vio->get_good_features_MSCKF();
        while(features.size() > points) {
            visualization.createPoint(points);
            points++;
        }
        for(int i = 0; i < features.size(); i++) {
            visualization.movePoint(i, features[i].data());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
