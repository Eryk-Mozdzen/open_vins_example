#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <unistd.h>
#include <vector>

#include "Visualization.hpp"

Visualization::Visualization(const std::string &ip) : points{0} {
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1) {
        sock = 0;
        return;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr(ip.c_str());

    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) == -1) {
        sock = 0;
    }

    write("{\"command\" : \"clear\"}");
    write("{\"command\" : \"config\", \"theme\" : \"dark\", \"camera\" : \"orbit\"}");
    write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [0.2, 0.2, 0.2]}, \
        \"material\" : {\"color\" : [128, 128, 128]}, \
        \"transform\" : {\"translation\" : [0, 0, 0]} \
    }");
    write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker/x\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [1, 0.05, 0.05]}, \
        \"material\" : {\"color\" : [255, 0, 0]}, \
        \"transform\" : {\"translation\" : [0.5, 0, 0]} \
    }");
    write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker/y\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [0.05, 1, 0.05]}, \
        \"material\" : {\"color\" : [0, 255, 0]}, \
        \"transform\" : {\"translation\" : [0, 0.5, 0]} \
    }");
    write("{ \
        \"command\" : \"create\", \
        \"path\" : \"marker/z\", \
        \"geometry\" : {\"shape\" : \"cuboid\", \"size\" : [0.05, 0.05, 1]}, \
        \"material\" : {\"color\" : [0, 0, 255]}, \
        \"transform\" : {\"translation\" : [0, 0, 0.5]} \
    }");
}

Visualization::~Visualization() {
    if(sock) {
        close(sock);
    }
}

void Visualization::write(const std::string &message) {
    if(sock) {
        send(sock, message.c_str(), message.size(), 0);
    }
}

void Visualization::update(const Eigen::Vector4d &attitude,
                           const Eigen::Vector3d &position,
                           const std::vector<Eigen::Vector3d> &features) {
    std::stringstream ss;
    ss << "{";
    ss << "\"command\" : \"update\",";
    ss << "\"path\" : \"marker\",";
    ss << "\"transform\" : {";
    ss << "\"translation\" : [";
    ss << position(0) << ",";
    ss << position(1) << ",";
    ss << position(2) << "],";
    ss << "\"quaternion\" : [";
    ss << attitude(3) << ",";
    ss << attitude(0) << ",";
    ss << attitude(1) << ",";
    ss << attitude(2) << "]";
    ss << "}";
    ss << "}";
    write(ss.str());

    std::stringstream ss2;
    ss2 << "{";
    ss2 << "\"command\" : \"camera\",";
    ss2 << "\"alpha\": 0.03,";
    ss2 << "\"position\" : [";
    ss2 << position(0) << ",";
    ss2 << position(1) << ",";
    ss2 << position(2) << "]";
    ss2 << "}";
    write(ss2.str());

    while(features.size() > points) {
        std::stringstream ss;
        ss << "{";
        ss << "\"command\" : \"create\",";
        ss << "\"path\" : \"point_" << points << "\",";
        ss << "\"geometry\" : {\"shape\" : \"sphere\", \"radius\" : 0.05},";
        ss << "\"material\" : {\"color\" : [255, 255, 255]},";
        ss << "\"transform\" : {\"translation\" : [0, 0, 0]}";
        ss << "}";
        write(ss.str());

        points++;
    }

    for(int i = 0; i < features.size(); i++) {
        std::stringstream ss;
        ss << "{";
        ss << "\"command\" : \"update\",";
        ss << "\"path\" : \"point_" << i << "\",";
        ss << "\"transform\" : {";
        ss << "\"translation\" : [";
        ss << features[i](0) << ",";
        ss << features[i](1) << ",";
        ss << features[i](2) << "]";
        ss << "}";
        ss << "}";
        write(ss.str());
    }
}
