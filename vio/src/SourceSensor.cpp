#include <fcntl.h>
#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

#include "SourceSensor.hpp"

struct MPU6050 {
    int64_t timestamp;
    int16_t gyro[3];
    int16_t accel[3];
    uint8_t _padding[4];
};

SourceSensor::SourceSensor(Source::Listener *listener)
    : Source(listener),
      threadIMU(&SourceSensor::readIMU, this),
      threadCAM(&SourceSensor::readCAM, this) {
}

void SourceSensor::readIMU() {
    const int fd = open("/dev/mpu6050", O_RDONLY | O_NONBLOCK);
    if(fd < 0) {
        std::cerr << "Device /dev/mpu6050 not found" << std::endl;
        return;
    }

    while(true) {
        MPU6050 mpu6050;
        if(read(fd, &mpu6050, sizeof(mpu6050)) != sizeof(mpu6050)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        constexpr const float scale_gyro = 0.017453292519943f / 65.5f;
        constexpr const float scale_accel = 9.80665f / 8192.f;

        Source::IMU sample;
        sample.timestamp = mpu6050.timestamp;
        sample.gyro[0] = mpu6050.gyro[0] * scale_gyro;
        sample.gyro[1] = mpu6050.gyro[1] * scale_gyro;
        sample.gyro[2] = mpu6050.gyro[2] * scale_gyro;
        sample.accel[0] = mpu6050.accel[0] * scale_accel;
        sample.accel[1] = mpu6050.accel[1] * scale_accel;
        sample.accel[2] = mpu6050.accel[2] * scale_accel;

        listener->available(sample);
    }
}

void SourceSensor::readCAM() {
    libcamera::CameraManager cm;
    cm.start();

    if(cm.cameras().empty()) {
        std::cerr << "No cameras found" << std::endl;
        return;
    }

    auto cam = cm.cameras()[0];
    cam->acquire();

    auto config = cam->generateConfiguration({libcamera::StreamRole::Viewfinder});
    config->at(0).pixelFormat = libcamera::formats::YUV420;
    config->at(0).size = {640, 480};
    config->at(0).bufferCount = 4;
    cam->configure(config.get());

    libcamera::FrameBufferAllocator allocator(cam);
    for(auto &cfg : *config) {
        if(allocator.allocate(cfg.stream()) < 0) {
            std::cerr << "Failed to allocate buffers" << std::endl;
            return;
        }
    }

    cam->start();

    while(true) {
        auto request = cam->createRequest();
        auto &stream = config->at(0).stream();
        const auto &buffers = allocator.buffers(stream);
        request->addBuffer(stream, buffers[0].get());

        cam->queueRequest(request.get());
        request = cam->waitForRequest();
        if(!request) {
            continue;
        }

        auto &buf = request->buffers().begin()->second->planes();

        uint8_t *yPlane = static_cast<uint8_t *>(
            mmap(nullptr, buf[0].length, PROT_READ | PROT_WRITE, MAP_SHARED, buf[0].fd.get(), 0));
        uint8_t *uvPlane = static_cast<uint8_t *>(
            mmap(nullptr, buf[1].length, PROT_READ | PROT_WRITE, MAP_SHARED, buf[1].fd.get(), 0));

        cv::Mat yuv(480 + 480 / 2, 640, CV_8UC1);
        memcpy(yuv.data, yPlane, buf[0].length);
        memcpy(yuv.data + 640 * 480, uvPlane, buf[1].length);

        Source::CAM sample;
        sample.timestamp = request->metadata().timestamp;
        cv::cvtColor(yuv, sample.img0, cv::COLOR_YUV2BGR_I420);

        listener->available(sample);

        munmap(yPlane, buf[0].length);
        munmap(uvPlane, buf[1].length);
    }
}
