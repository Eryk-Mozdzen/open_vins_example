#include <arpa/inet.h>
#include <sstream>
#include <string>
#include <unistd.h>

#include "Visualization.hpp"

Visualization::Visualization() {
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock == -1) {
        sock = 0;
        return;
    }

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("10.42.0.1");

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

void Visualization::write(const std::string message) {
    if(sock) {
        send(sock, message.c_str(), message.size(), 0);
    }
}

void Visualization::update(const double quaternion[4], const double position[3]) {
    std::stringstream ss;
    ss << "{";
    ss << "\"command\" : \"update\",";
    ss << "\"path\" : \"marker\",";
    ss << "\"transform\" : {";
    //ss << "\"translation\" : [";
    //ss << position[0] << ", ";
    //ss << position[1] << ", ";
    //ss << position[2] << "],";
    ss << "\"quaternion\" : [";
    ss << quaternion[3] << ", ";
    ss << quaternion[0] << ", ";
    ss << quaternion[1] << ", ";
    ss << quaternion[2] << "]";
    ss << "}";
    ss << "}";
    write(ss.str());

    //std::stringstream ss2;
    //ss2 << "{";
    //ss2 << "\"command\" : \"camera\",";
    //ss2 << "\"alpha\": 0.03,";
    //ss2 << "\"position\" : [";
    //ss2 << position[0] << ", ";
    //ss2 << position[1] << ", ";
    //ss2 << position[2] << "]";
    //ss2 << "}";
    //write(ss2.str());
}
