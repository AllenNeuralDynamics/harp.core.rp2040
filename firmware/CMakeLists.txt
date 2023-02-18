cmake_minimum_required(VERSION 3.13)

include(pico-sdk/pico_sdk_init.cmake)

project(mis_focus_controller)

# Enable try/catch exceptions. (Required for string parsing implementation.)
set(PICO_CXX_ENABLE_EXCEPTIONS 1)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(${PROJECT_NAME}
    src/main.cpp
)

add_library(motor_controller
    src/motor_controller.cpp
)

add_library(en_dir_motor_driver
    src/en_dir_motor_driver.cpp
)

add_library(encoder
    src/cpu_encoder.cpp
)

add_library(user_io_handler
    src/user_io_handler.cpp
)

add_library(core1_main
    src/core1_main.cpp
)


# Where to look for header files.
#target_include_directories(${PROJECT_NAME} PUBLIC inc)

# Where to look for header files.
#target_include_directories(brushed_motor_controller PUBLIC inc)
include_directories(inc)


# For Debugging.
# Check how many CPU cycles an encoder reading iteration takes.
#add_definitions(-DPROFILE_ENCODER_LOOP)
# Print encoder ticks values.
#add_definitions(-DENCODER_DEBUG)
# Print state change information.
#add_definitions(-DDEBUG)


# Link libraries to the targets that need them.
target_link_libraries(motor_controller en_dir_motor_driver encoder)
target_link_libraries(en_dir_motor_driver pico_stdlib hardware_pwm)
target_link_libraries(user_io_handler pico_stdlib motor_controller)
target_link_libraries(core1_main pico_stdlib)
target_link_libraries(${PROJECT_NAME}
                      pico_stdlib motor_controller user_io_handler
                      pico_multicore core1_main)

# create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(${PROJECT_NAME})

# USB serial
pico_enable_stdio_usb(${PROJECT_NAME} 1)
pico_enable_stdio_uart(${PROJECT_NAME} 0)

pico_enable_stdio_usb(user_io_handler 1)
pico_enable_stdio_uart(user_io_handler 0)
