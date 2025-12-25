#ifdef RECORD
#include "ListenerRecord.hpp"
#include "SourceHardware.hpp"

int main(int argc, char **argv) {
    if(argc != 3) {
        std::cerr << "USAGE:   " << argv[0] << " <dataset path> <duration in seconds>\n\n"
                  << "EXAMPLE: " << argv[0] << " datasets/custom 20" << std::endl;
        return -1;
    }

    ListenerRecord listener(argv[1]);
    SourceHardware source(listener);
    std::this_thread::sleep_for(std::chrono::seconds(std::stoul(argv[2])));
}
#endif

#ifdef VIO_OFFLINE
#include "ListenerOpenVINS.hpp"
#include "SourceReplay.hpp"
#include "Visualization.hpp"

int main(int argc, char **argv) {
    if(argc != 4) {
        std::cerr << "USAGE:   " << argv[0]
                  << " <dataset path> <config path> <visualization server IP>\n\n"
                  << "EXAMPLE: " << argv[0]
                  << " datasets/dataset-corridor1_512_16 config/tum/config.yaml 192.168.0.17\n"
                  << "         " << argv[0]
                  << " datasets/MH_01_easy               config/eth/config.yaml 192.168.0.17"
                  << std::endl;
        return -1;
    }

    ListenerOpenVINS listener(argv[2]);
    SourceReplay source(listener, argv[1]);
    Visualization visualization(argv[3]);

    while(source.available()) {
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
#endif

#ifdef VIO_ONLINE
#include "ListenerOpenVINS.hpp"
#include "SourceHardware.hpp"
#include "Visualization.hpp"

int main(int argc, char **argv) {
    if(argc != 3) {
        std::cerr << "USAGE:   " << argv[0] << " <config path> <visualization server IP>\n\n"
                  << "EXAMPLE: " << argv[0] << " config/custom/config.yaml 192.168.0.17"
                  << std::endl;
        return -1;
    }

    ListenerOpenVINS listener(argv[1]);
    SourceHardware source(listener);
    Visualization visualization(argv[2]);

    while(source.available()) {
        const Eigen::Vector4d attitude = listener.getAttitude();
        const Eigen::Vector3d position = listener.getPosition();
        const std::vector<Eigen::Vector3d> features = listener.getFeatures();

        visualization.update(attitude, position, features);

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
#endif
