#include "ListenerOpenVINS.hpp"
#include "ListenerRecord.hpp"
#include "Source.hpp"
#include "SourceHardware.hpp"
#include "SourceReplay.hpp"
#include "Visualization.hpp"

int main() {
    // ListenerRecord listener("../datasets/custom");
    ListenerOpenVINS listener("../config/tum/config.yaml");
    // ListenerOpenVINS listener("../config/euroc/config.yaml");
    // ListenerOpenVINS listener("../config/custom/config.yaml");

    SourceReplay source(listener, "../datasets/dataset-corridor1_512_16");
    // SourceReplay source(listener, "../datasets/MH_01_easy");
    // SourceReplay source(listener, "../datasets/custom");
    // SourceHardware source(listener);

    // std::this_thread::sleep_for(std::chrono::seconds(20));

    Visualization visualization("192.168.168.50");

    while(true) {
        const cv::Mat image = listener.getImage();

        if(!image.empty()) {
            cv::imshow("image", image);
            cv::waitKey(1);
        }

        const Eigen::Vector4d attitude = listener.getAttitude();
        const Eigen::Vector3d position = listener.getPosition();
        const std::vector<Eigen::Vector3d> features = listener.getFeatures();

        visualization.update(attitude, position, features);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
