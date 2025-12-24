cmake_minimum_required(VERSION 3.20)

project(example)

find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(Ceres REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED IMPORTED_TARGET libcamera)

add_executable(record
    "src/main.cpp"
    "src/ListenerRecord.cpp"
    "src/Source.cpp"
    "src/SourceHardware.cpp"
)

add_executable(vio_offline
    "src/main.cpp"
    "src/ListenerOpenVINS.cpp"
    "src/Source.cpp"
    "src/SourceReplay.cpp"
    "src/Visualization.cpp"
)

add_executable(vio_online
    "src/main.cpp"
    "src/ListenerOpenVINS.cpp"
    "src/Source.cpp"
    "src/SourceHardware.cpp"
    "src/Visualization.cpp"
)

target_include_directories(record PRIVATE
    "include"
    ${LIBCAMERA_INCLUDE_DIRS}
)

target_include_directories(vio_offline PRIVATE
    "include"
    "/usr/local/include/open_vins"
)

target_include_directories(vio_online PRIVATE
    "include"
    ${LIBCAMERA_INCLUDE_DIRS}
    "/usr/local/include/open_vins"
)

target_compile_options(record PRIVATE
    -O3
    -DRECORD
)

target_compile_options(vio_offline PRIVATE
    -O3
    -DVIO_OFFLINE
)

target_compile_options(vio_online PRIVATE
    -O3
    -DVIO_ONLINE
)

target_link_libraries(record PRIVATE
    ${OpenCV_LIBRARIES}
    PkgConfig::LIBCAMERA
)

target_link_libraries(vio_offline PRIVATE
    Boost::filesystem
    Boost::system
    Eigen3::Eigen
    ${OpenCV_LIBRARIES}
    ${CERES_LIBRARIES}
    "/usr/local/lib/libov_msckf_lib.so"
)

target_link_libraries(vio_online PRIVATE
    Boost::filesystem
    Boost::system
    Eigen3::Eigen
    ${OpenCV_LIBRARIES}
    ${CERES_LIBRARIES}
    PkgConfig::LIBCAMERA
    "/usr/local/lib/libov_msckf_lib.so"
)
