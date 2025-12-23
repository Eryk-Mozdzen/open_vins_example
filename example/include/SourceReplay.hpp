#ifndef SOURCE_REPLAY_HPP
#define SOURCE_REPLAY_HPP

#include <chrono>
#include <thread>

#include "Source.hpp"

class SourceReplay : public Source {
    std::string path;
    int64_t reference;
    std::chrono::system_clock::time_point start;
    std::thread threadIMU;
    std::thread threadCAM;

    void readIMU();
    void readCAM();

public:
    SourceReplay(Source::Listener &listener, const std::string path);
};

#endif
