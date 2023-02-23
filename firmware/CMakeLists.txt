cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(harp_core_rp2040)

# Enable try/catch exceptions. (Required for string parsing implementation.)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# Use modern conventions like std::invoke
set(CMAKE_CXX_STANDARD 17)


# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.cpp
)


add_library(registers
    src/registers.cpp
)

add_library(harp_core
    src/harp_core.cpp
)

#add_library(core1_main
#    src/core1_main.cpp
#)


# Where to look for header files.
#target_include_directories(${PROJECT_NAME} PUBLIC inc)

# Where to look for header files.
#target_include_directories(brushed_motor_controller PUBLIC inc)
include_directories(inc)


# For Debugging.
# Print state change information.
#add_definitions(-DDEBUG)


# Link libraries to the targets that need them.
#target_link_libraries(<target> <lib0> <lib1>)
#target_link_libraries(core1_main pico_stdlib)
target_link_libraries(harp_core registers)
target_link_libraries(${PROJECT_NAME} pico_stdlib registers harp_core)
#target_link_libraries(${PROJECT_NAME}
#                      pico_stdlib motor_controller user_io_handler
#                      pico_multicore core1_main)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

# USB serial
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

#pico_enable_stdio_usb(user_io_handler 1)
#pico_enable_stdio_uart(user_io_handler 0)
