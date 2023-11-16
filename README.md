# Harp Core RP2040

An RP2040 Harp Core that implements the [Harp Protocol](https://github.com/harp-tech/protocol) to serve as the basis of a custom Harp device.

## Features
* Synchronization to an external Harp Clock Synchronizer signal.
* Parsing incoming harp messages
* Dispatching messages to the appropriate register
* Sending harp-compliant timestamped replies

## Examples
See the [examples](./examples) folder to get a feel for incorporating the harp core into your own project.

Additionally, are a few examples that use the RP2040 Harp Core as a submodule in the wild:
* [harp.device.environment-sensor](https://github.com/AllenNeuralDynamics/harp.device.environment_sensor)
* [harp.device.lickety-split](https://github.com/AllenNeuralDynamics/harp.device.lickety-split)
* [harp.device.treadmill](https://github.com/AllenNeuralDynamics/harp.device.treadmill)
* [harp.device.ttl-io](https://github.com/AllenNeuralDynamics/harp.device.ttl-io)

---
# Using this Library
The easiest way to use this library is to include it as submodule in your project.
To see how to structure your project to incorporate the RP2040 Harp Core as a library, see the examples above--or read on.

## Install the Pico SDK
Download (or clone) the [Pico SDK](https://github.com/raspberrypi/pico-sdk) to a known folder on your PC.
(This folder does not need to be a sub-folder of your project.)
From the Pico SDK project root folder, install the Pico SDK's dependencies with:
````
git submodule update --init --recursive
````
## Install the Harp Core as a submodule
Next, in a sub-folder of your project, add **harp.core.rp2040** as a submodule with:
````
git submodule add git@github.com:AllenNeuralDynamics/harp.core.rp2040.git
````

## Setup your Project's CMakeLists.txt
At the top of your CMakeLists.txt, you will need to include initialize the Pico SDK, and point to the folder of this library's CMakeLists.txt
````CMakeLists.txt
include(${PICO_SDK_PATH}/pico_sdk_init.cmake)
pico_sdk_init()
add_subdirectory(/path/to/cmakelist_dir build) # Path to harp.core.rp2040's CMakeLists.txt
````
After you've setup the recipe to build your executable, you can link against the Harp core libraries with:
````CMakeLists.txt
target_link_libraries(${PROJECT_NAME} harp_core harp_sync)
````

## Point to the Pico SDK
Recommended, but optional: define the `PICO_SDK_PATH` environment variable to point to the location where the pico-sdk was downloaded. i.e:
````
PICO_SDK_PATH=/home/username/projects/pico-sdk
````
On Linux, it may be preferrable to put this in your `.bashrc` file.

## Compiling the Firmware

### Without an IDE
From this directory, create a directory called build, enter it, and invoke cmake with:
````
mkdir build
cd build
cmake ..
````
If you did not define the `PICO_SDK_PATH` as an environment variable, you must pass it in here like so:
````
mkdir build
cd build
cmake -DPICO_SDK_PATH=/path/to/pico-sdk ..
````
After this point, you can invoke the auto-generated Makefile with `make`

## Flashing the Firmware
Press-and-hold the Pico's BOOTSEL button and power it up (i.e: plug it into usb).
At this point you do one of the following:
* drag-and-drop the created **\*.uf2** file into the mass storage device that appears on your pc.
* flash with [picotool](https://github.com/raspberrypi/picotool)

---
# Developer Notes

### Debugging with printf
The Harp Core consumes the USB serial port, so `printf` messages must be rerouted to an available UART port.
The Pico SDK makes this step fairly straightforward. Before calling `printf` you must first setup a UART port with:
````C
#define UART_TX_PIN (0)
#define BAUD_RATE (921600)
stdio_uart_init_full(uart0, BAUD_RATE, UART_TX_PIN, -1) // or uart1, depending on pin
````
To read these messages, you must connect a [3.3V FTDI Cable](https://www.digikey.com/en/products/detail/adafruit-industries-llc/954/7064488?) to the corresponding pin and connect to it with the matching baud rate.

Additionally, in your CMakeLists.txt, you must add     
````cmake
pico_enable_stdio_uart(${PROJECT_NAME} 1) # UART stdio for printf.
````
for each library and executable using `printf` and you must link it with `pico_stdlib`.

### Debugging the Core
`printf` messages are sprinkled throughout the Harp Core code, and they can be conditionally compiled by adding flags to your CMakeLists.txt.

To print out details of every *outgoing* (Device to PC) messages, add:
````cmake
add_definitions(-DDEBUG_HARP_MSG_OUT)
````

To print out details of every *received* (PC to Device) messages, add:
````cmake
add_definitions(-DDEBUG_HARP_MSG_IN)
````

# References
* [Harp Protocol Repo](https://github.com/harp-tech/protocol)
* [pyharp](https://github.com/harp-tech/pyharp) python library for connecting to harp-compliant devices and sending read/writes.
