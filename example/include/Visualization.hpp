#ifndef VISUALIZATION_HPP
#define VISUALIZATION_HPP

#include <string>
#include <vector>

#include <Eigen/Dense>

class Visualization {
    int sock;
    int points;

    void write(const std::string &message);

public:
    Visualization(const std::string &ip);
    ~Visualization();

    void update(const Eigen::Vector4d &attitude,
                const Eigen::Vector3d &position,
                const std::vector<Eigen::Vector3d> &features);
};

#endif
