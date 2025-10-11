cmake_minimum_required(VERSION 3.20)

project(calibrate)

find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)
pkg_check_modules(LIBCAMERA REQUIRED libcamera)

add_executable(${CMAKE_PROJECT_NAME}
    "main.cpp"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    ${LIBCAMERA_INCLUDE_DIRS}
)

target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE
    -O3
)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE
    ${OpenCV_LIBRARIES}
    ${LIBCAMERA_LIBRARIES}
)
