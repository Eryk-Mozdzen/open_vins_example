cmake_minimum_required(VERSION 3.20)

project(vio)

find_package(PkgConfig REQUIRED)
find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(Ceres REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

add_executable(${CMAKE_PROJECT_NAME}
    "src/main.cpp"
    "src/Source.cpp"
    "src/SourceDataset.cpp"
    "src/SourceHardware.cpp"
    "src/Visualization.cpp"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "include"
    ${LIBCAMERA_INCLUDE_DIRS}
    "/usr/local/include/open_vins"
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -O3
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Boost::filesystem
    Boost::system
    Eigen3::Eigen
    ${OpenCV_LIBRARIES}
    ${CERES_LIBRARIES}
    ${LIBCAMERA_LIBRARIES}
    "/usr/local/lib/libov_msckf_lib.so"
)
