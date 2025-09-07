#ifndef SOURCE_ANDROID_HPP
#define SOURCE_ANDROID_HPP

#include <thread>

#include "Source.hpp"

class SourceAndroid : public Source {
    std::thread thread;

    void read();

public:
    SourceAndroid(Source::Listener *listener);
};

#endif
