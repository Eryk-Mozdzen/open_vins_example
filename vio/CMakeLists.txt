cmake_minimum_required(VERSION 3.20)

project(vio)

find_package(Boost REQUIRED COMPONENTS filesystem system)
find_package(Ceres REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(OpenVINS REQUIRED)

add_executable(${CMAKE_PROJECT_NAME}
    "src/main.cpp"
    "src/Source.cpp"
    "src/SourceDataset.cpp"
    "src/SourceAndroid.cpp"
    "src/Visualization.cpp"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "include"

    #${OPENVINS_INCLUDE_DIR}

    "/usr/local/include/open_vins"
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    #-O3
    #-Wall
    #-Wextra
    #-Wpedantic
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    Boost::filesystem
    Boost::system
    Eigen3::Eigen
    ${OpenCV_LIBS}
    ${CERES_LIBRARIES}
    #${OPENVINS_LIBRARIES}

    "/usr/local/lib/libov_msckf_lib.so"
)
