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

add_library(core_registers
    src/core_registers.cpp
)

add_library(usb_desc
    src/usb_descriptors.c
)

add_library(harp_core
    src/harp_core.cpp
)

add_library(harp_sync
    src/harp_synchronizer.cpp
)

# Where to look for header files.
#target_include_directories(brushed_motor_controller PUBLIC inc)
include_directories(inc)

# For Debugging.
#add_definitions(-DDEBUG)

target_link_libraries(usb_desc tinyusb_device pico_unique_id)
target_link_libraries(harp_sync pico_stdlib)
target_link_libraries(harp_core core_registers pico_stdlib tinyusb_device usb_desc)
target_link_libraries(${PROJECT_NAME} pico_stdlib harp_core harp_sync)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

if(DEFINED <DEBUG>)
    # UART stdio for printf.
    pico_enable_stdio_uart(${PROJECT_NAME} 1)
    # Additional libraries need to have stdio init also.
    pico_enable_stdio_uart(harp_core 1)
    pico_enable_stdio_uart(harp_sync 1)
endif()
