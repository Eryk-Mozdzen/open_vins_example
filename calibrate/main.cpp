#include <chrono>
#include <fcntl.h>
#include <mutex>
#include <sys/mman.h>
#include <thread>
#include <unistd.h>

#include <libcamera/libcamera.h>
#include <opencv2/opencv.hpp>

std::shared_ptr<libcamera::Camera> camera;
cv::Mat image;
std::mutex mutex;

void ready(libcamera::Request *request) {
    if(request->status() == libcamera::Request::RequestCancelled) {
        return;
    }

    for(auto &[stream, buffer] : request->buffers()) {
        const auto &plane = buffer->planes().front();
        void *data = mmap(nullptr, plane.length, PROT_READ, MAP_SHARED, plane.fd.get(), 0);

        const cv::Mat gray(480, 640, CV_8UC1, data);
        mutex.lock();
        image = gray.clone();
        mutex.unlock();

        munmap(data, plane.length);
    }

    request->reuse(libcamera::Request::ReuseBuffers);
    camera->queueRequest(request);
}

int main() {
    std::unique_ptr<libcamera::CameraManager> cm = std::make_unique<libcamera::CameraManager>();
    cm->start();

    if(cm->cameras().empty()) {
        cm->stop();
        std::cerr << "No cameras found" << std::endl;
        return 0;
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
            return 0;
        }
    }

    camera->start();
    camera->requestCompleted.connect(ready);

    auto request = camera->createRequest();
    for(auto &streamConfig : *config) {
        auto stream = streamConfig.stream();
        auto &buffers = allocator.buffers(stream);
        request->addBuffer(stream, buffers[0].get());
    }

    camera->queueRequest(request.get());

    /*cv::TermCriteria criteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.001);

    std::vector<cv::Point3f> objp;
    for(int i = 0; i < 7; ++i) {
        for(int j = 0; j < 7; ++j) {
            objp.push_back(cv::Point3f(j * 0.02f, i * 0.02f, 0.0f));
        }
    }

    std::vector<std::vector<cv::Point3f>> objpoints;
    std::vector<std::vector<cv::Point2f>> imgpoints;*/

    while(true) {
        mutex.lock();
        cv::Mat copy = image.clone();
        mutex.unlock();

        if(copy.empty()) {
            continue;
        }

        /*std::vector<cv::Point2f> corners;
        bool found =
            cv::findChessboardCorners(copy, cv::Size(7, 7), corners,
                                      cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE);

        if(found) {
            cv::cornerSubPix(copy, corners, cv::Size(11, 11), cv::Size(-1, -1), criteria);

            objpoints.push_back(objp);
            imgpoints.push_back(corners);

            cv::drawChessboardCorners(copy, cv::Size(7, 7), corners, found);
        }

        if(imgpoints.size() > 10) {
            objpoints.erase(objpoints.begin(), objpoints.end() - 10);
            imgpoints.erase(imgpoints.begin(), imgpoints.end() - 10);

            cv::Mat cameraMatrix;
            cv::Mat distCoeffs;
            std::vector<cv::Mat> rvecs;
            std::vector<cv::Mat> tvecs;

            cv::calibrateCamera(objpoints, imgpoints, copy.size(), cameraMatrix, distCoeffs, rvecs,
                                tvecs);

            std::cout << "Distortion coefficients:\n" << distCoeffs << std::endl;
            std::cout << "Camera matrix:\n" << cameraMatrix << std::endl;
        }*/

        cv::imshow("window", copy);
        cv::waitKey(1);
    }
}
