#include <fcntl.h>
#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

#include "SourceHardware.hpp"

// match MPU6050 kernel module structure
struct MPU6050 {
    int64_t timestamp;
    int16_t gyro[3];
    int16_t accel[3];
};

SourceHardware::SourceHardware(Source::Listener *listener)
    : Source(listener),
      threadIMU(&SourceHardware::readIMU, this),
      threadCAM(&SourceHardware::readCAM, this) {
}

void SourceHardware::readIMU() {
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

        const int64_t timestamp = mpu6050.timestamp;

        const float accel[3] = {
            mpu6050.accel[0] * scale_accel,
            mpu6050.accel[1] * scale_accel,
            mpu6050.accel[2] * scale_accel,
        };

        const float gyro[3] = {
            mpu6050.gyro[0] * scale_gyro,
            mpu6050.gyro[1] * scale_gyro,
            mpu6050.gyro[2] * scale_gyro,
        };

        listener.handle(timestamp, accel, gyro);
    }
}

void SourceHardware::readCAM() {
    std::unique_ptr<libcamera::CameraManager> cm = std::make_unique<libcamera::CameraManager>();
    cm->start();

    if(cm->cameras().empty()) {
        cm->stop();
        std::cerr << "No cameras found" << std::endl;
        return;
    }

    camera = cm->get(cm->cameras()[0]->id());
    camera->acquire();

    auto config = camera->generateConfiguration({libcamera::StreamRole::Viewfinder});
    config->at(0).pixelFormat = libcamera::formats::YUV420;
    config->at(0).size.width = 640;
    config->at(0).size.height = 480;
    config->at(0).bufferCount = 4;
    config->validate();
    camera->configure(config.get());

    libcamera::FrameBufferAllocator allocator(camera);
    for(auto &cfg : *config) {
        if(allocator.allocate(cfg.stream()) < 0) {
            camera->release();
            camera.reset();
            cm->stop();
            std::cerr << "Failed to allocate buffers" << std::endl;
            return;
        }
    }

    camera->start();
    camera->requestCompleted.connect(this, &SourceHardware::readyCAM);

    auto request = camera->createRequest();
    for(auto &streamConfig : *config) {
        auto stream = streamConfig.stream();
        auto &buffers = allocator.buffers(stream);
        request->addBuffer(stream, buffers[0].get());
    }

    auto &controls = request->controls();
    controls.set(libcamera::controls::ExposureTimeMode,
                 libcamera::controls::ExposureTimeModeManual);
    controls.set(libcamera::controls::ExposureTime, 10000);
    controls.set(libcamera::controls::FrameDurationLimits,
                 libcamera::Span<const int64_t, 2>({50000, 50000}));

    camera->queueRequest(request.get());

    while(true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void SourceHardware::readyCAM(libcamera::Request *request) {
    if(request->status() == libcamera::Request::RequestCancelled) {
        return;
    }

    const int64_t timestamp =
        request->metadata().get(libcamera::controls::SensorTimestamp).value_or(0);

    cv::Mat img;

    for(auto &[stream, buffer] : request->buffers()) {
        const auto &plane = buffer->planes().front();
        void *data = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

        const cv::Mat gray(480, 640, CV_8UC1, data);

        // my hardware setup consist of camera rotated by 180deg, so I need to rotate image back
        cv::flip(gray, img, -1);

        munmap(data, plane.length);
    }

    listener.handle(timestamp, img);

    request->reuse(libcamera::Request::ReuseBuffers);
    camera->queueRequest(request);
}
