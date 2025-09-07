#include <arpa/inet.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>

#include "SourceAndroid.hpp"

SourceAndroid::SourceAndroid(Source::Listener *listener)
    : Source(listener), thread(&SourceAndroid::read, this) {
}

void SourceAndroid::read() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0) {
        return;
    }

    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(sockfd);
        return;
    }

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(4444);

    if(bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sockfd);
        return;
    }

    uint8_t buffer[2*307200];

    while(true) {
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&addr, &addrlen);
        if(n < 0) {
            break;
        }

        if(n == sizeof(Source::IMU)) {
            Source::IMU *sample = reinterpret_cast<Source::IMU *>(buffer);

            // printf("IMU %20lld %+10.3f %+10.3f %+10.3f %+10.3f %+10.3f %+10.3f\n",
            //        (long long)sample->timestamp, sample->accel[0], sample->accel[1],
            //        sample->accel[2], sample->gyro[0], sample->gyro[1], sample->gyro[2]);

            listener->available(*sample);
        } else {
            printf("%lu\n", n);

            // listener->available(*sample);
        }
    }
}
