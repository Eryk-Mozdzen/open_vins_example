#ifndef SOURCE_SENSOR_HPP
#define SOURCE_SENSOR_HPP

#include <thread>

#include "Source.hpp"

class SourceSensor : public Source {
    std::thread threadIMU;
    std::thread threadCAM;

    void readIMU();
    void readCAM();

public:
    SourceSensor(Source::Listener *listener);
};

#endif
