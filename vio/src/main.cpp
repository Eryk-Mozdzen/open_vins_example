#include <core/VioManager.h>
#include <state/State.h>

#include "Source.hpp"
// #include "SourceDataset.hpp"
#include "SourceSensor.hpp"
#include "Visualization.hpp"

class Listener : public Source::Listener {
    std::shared_ptr<ov_msckf::VioManager> vio;

    void available(const Source::IMU &sample) override {
        ov_core::ImuData data;
        data.timestamp = sample.timestamp * 1E-9;
        data.wm = Eigen::Vector3d(sample.gyro[0], sample.gyro[1], sample.gyro[2]);
        data.am = Eigen::Vector3d(sample.accel[0], sample.accel[1], sample.accel[2]);

        vio->feed_measurement_imu(data);
    }

    void available(const Source::CAM &sample) override {
        printf("%20lld\n", (long long)sample.timestamp);

        cv::Mat img0;
        cv::cvtColor(sample.img0, img0, cv::COLOR_BGR2GRAY);

        ov_core::CameraData data;
        data.timestamp = sample.timestamp * 1E-9;
        data.sensor_ids = {0};
        data.masks = {cv::Mat(640, 480, CV_8UC1, cv::Scalar(0))};
        data.images = {img0};

        vio->feed_measurement_camera(data);
    }

public:
    Listener(std::shared_ptr<ov_msckf::VioManager> vio) : vio{vio} {
    }
};

int main() {
    const auto parser = std::make_shared<ov_core::YamlParser>("../config.yaml");

    ov_msckf::VioManagerOptions params;
    params.print_and_load(parser);

    auto vio = std::make_shared<ov_msckf::VioManager>(params);

    Listener listener(vio);

    // SourceDataset source(&listener, "datasets/dataset-corridor1_512_16");
    SourceSensor source(&listener);

    Visualization visualization;

    while(true) {
        /*double timestamp;
        cv::Mat window;

        vio->get_active_image(timestamp, window);

        if((window.size().width > 0) && (window.size().height > 0)) {
            cv::imshow("window", window);
            cv::waitKey(1);
        }*/

        const auto q = vio->get_state()->_imu->quat();
        const auto p = vio->get_state()->_imu->pos();

        visualization.update(q.data(), p.data());

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
}
