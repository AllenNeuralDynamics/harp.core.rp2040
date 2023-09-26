cmake_minimum_required(VERSION 3.13)

# PICO_SDK_PATH must be defined.
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)

project(harp_c_app_example)

pico_sdk_init()
add_subdirectory(../../firmware build) # Path to harp.core.rp2040.

add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Use modern conventions like std::invoke
set(CMAKE_CXX_STANDARD 17)

include_directories(inc)

target_link_libraries(${PROJECT_NAME} harp_c_app harp_sync)

pico_add_extra_outputs(${PROJECT_NAME})
