FROM debian:bookworm

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y \
        build-essential \
        wget \
        git \
        cmake

RUN wget https://developer.arm.com/-/media/Files/downloads/gnu/12.2.rel1/binrel/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz \
    && tar -xf arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu.tar.xz

ENV PATH="$PATH:/arm-gnu-toolchain-12.2.rel1-x86_64-aarch64-none-linux-gnu/bin"

RUN mkdir -p /install

COPY toolchain.cmake /toolchain.cmake

RUN wget https://github.com/boostorg/boost/releases/download/boost-1.89.0/boost-1.89.0-cmake.tar.xz \
    && tar -xf boost-1.89.0-cmake.tar.xz \
    && cd boost-1.89.0 \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
    && make -j4 \
    && make install

RUN git clone --depth 1 --branch 3.4.0 --recursive https://gitlab.com/libeigen/eigen.git \
    && cd eigen \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
    && make -j4 \
    && make install

RUN git clone --depth 1 --branch v2.2.2 --recursive https://github.com/gflags/gflags.git \
    && cd gflags \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
    && make -j4 \
    && make install

RUN git clone --depth 1 --branch v0.7.1 --recursive https://github.com/google/glog.git \
    && cd glog \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
    && make -j4 \
    && make install

RUN git clone --depth 1 --branch 2.1.0 --recursive https://github.com/ceres-solver/ceres-solver.git \
    && cd ceres-solver \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
    && make -j4 \
    && make install

RUN git clone --depth 1 --branch 4.12.0 --recursive https://github.com/opencv/opencv.git \
    && git clone --depth 1 --branch 4.12.0 --recursive https://github.com/opencv/opencv_contrib.git \
    && cd opencv \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
        -DOPENCV_EXTRA_MODULES_PATH=../../opencv_contrib/modules \
    && make -j4 \
    && make install

RUN git clone --depth 1 --branch v2.7 --recursive https://github.com/rpng/open_vins.git \
    && cd open_vins/ov_msckf \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/toolchain.cmake \
        -DCMAKE_INSTALL_PREFIX=/install \
        -DBUILD_SHARED_LIBS=ON \
        -DENABLE_ROS=OFF \
        -DENABLE_ARUCO_TAGS=OFF \
    && make -j4 \
    && make install

RUN git clone --depth=1 --branch rpi-6.12.y --recursive https://github.com/raspberrypi/linux.git \
    && cd linux \
    && apt-get install -y \
        flex \
        bison \
        bc \
        libssl-dev \
    && make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- bcm2712_defconfig \
    && make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules
