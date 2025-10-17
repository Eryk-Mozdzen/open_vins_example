#ifndef SOURCE_DATASET_HPP
#define SOURCE_DATASET_HPP

#include <chrono>
#include <thread>

#include "Source.hpp"

class SourceDataset : public Source {
    std::string path;
    int64_t reference;
    std::chrono::_V2::system_clock::time_point start;
    std::thread threadIMU;
    std::thread threadCAM;

    void readIMU();
    void readCAM();

public:
    SourceDataset(Source::Listener *listener, const std::string path);
};

#endif
