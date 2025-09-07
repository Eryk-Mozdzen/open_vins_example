#ifndef SOURCE_DATASET_HPP
#define SOURCE_DATASET_HPP

#include <thread>

#include "Source.hpp"

class SourceDataset : public Source {
    std::string path;
    std::thread threadIMU;
    std::thread threadCAM;

    void readIMU();
    void readCAM();

public:
    SourceDataset(Source::Listener *listener, const std::string path);
};

#endif
