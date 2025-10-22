# OpenVINS example project

### Prerequisites

```bash
sudo apt install -y \
    build-essential \
    git \
    cmake \
    linux-headers-$(uname -r) \
    libboost-all-dev \
    libeigen3-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libcxsparse4 \
    libopencv-dev \
    libcamera-dev \
    python3-opencv \
    python3-numpy
```

```bash
git clone --depth 1 --branch 2.1.0 --recursive https://github.com/ceres-solver/ceres-solver.git \
    && cd ceres-solver \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
    && make \
    && sudo make install
```

```bash
git clone --depth 1 --branch v2.7 https://github.com/rpng/open_vins.git \
    && cd open_vins/ov_msckf \
    && mkdir build \
    && cd build \
    && cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DBUILD_SHARED_LIBS=ON \
        -DENABLE_ROS=OFF \
        -DENABLE_ARUCO_TAGS=OFF \
    && make \
    && sudo make install
```

### IMU

```bash
sudo raspi-config
# Interface Options > I2C > Enable

sudo nano /etc/modprobe.d/blacklist-mpu6050.conf
# blacklist inv-mpu6050-i2c
# blacklist inv-mpu6050

cd mpu6050 \
    && make \
    && sudo insmmod mpu6050.ko
```
