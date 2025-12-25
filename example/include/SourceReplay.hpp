#ifndef SOURCE_REPLAY_HPP
#define SOURCE_REPLAY_HPP

#include <atomic>
#include <chrono>
#include <thread>

#include "Source.hpp"

class SourceReplay : public Source {
    std::string path;
    int64_t reference;
    std::chrono::system_clock::time_point start;
    std::thread threadIMU;
    std::thread threadCAM;
    std::atomic_bool flag;

    void readIMU();
    void readCAM();

public:
    SourceReplay(Source::Listener &listener, const std::string path);
    ~SourceReplay();

    bool available() const override;
};

#endif
