#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

class Visualization {
    int sock;

    void write(const std::string message);

public:
    Visualization();
    ~Visualization();

    void update(const double quaternion[4], const double position[3]);
};

#endif
