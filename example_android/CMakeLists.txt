cmake_minimum_required(VERSION 3.16)

set(ANDROID_PLATFORM 30)
set(ANDROID_ABI arm64-v8a)
set(ANDROID_NDK "/home/emozdzen/android-sdk/ndk/25.2.9519653")

set(CMAKE_TOOLCHAIN_FILE "${ANDROID_NDK}/build/cmake/android.toolchain.cmake")

project(app)

add_library(${CMAKE_PROJECT_NAME} SHARED
    "src/main.c"
    "${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c"
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${ANDROID_NDK}/sources/android/native_app_glue"
)

find_library(log log)
find_library(android android)

target_link_libraries(${CMAKE_PROJECT_NAME}
    ${log}
    ${android}
)
