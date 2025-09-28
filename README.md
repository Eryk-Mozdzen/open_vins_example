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
    libceres-dev \
    libopencv-dev \
    libcamera-dev
```

### Camera

```bash
sudo nano /boot/firmware/config.txt
# camera_auto_detect=0
# dtoverlay=ov5647,cam0   add to [all] section
```

### IMU

```bash
sudo raspi-config
# Interface Options > I2C > Enable

cd mpu6050 \
    && make \
    && sudo insmmod mpu6050.ko
```

### OpenVINS

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
