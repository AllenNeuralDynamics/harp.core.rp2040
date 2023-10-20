# Harp Core RP2040

An RP2040 Harp Core that implements the [Harp Protocol](https://github.com/harp-tech/protocol) to serve as the basis of a custom Harp device.


## Overview

TODO

## Usage

## Examples
See the examples folder to get a feel for incorporating the harp core into your own project.

Here are a few examples that use the RP2040 Harp Core in the wild:
* [harp.device.environment-sensor](https://github.com/AllenNeuralDynamics/harp.device.environment_sensor)
* [harp.device.lickety-split](https://github.com/AllenNeuralDynamics/harp.device.lickety-split)
* [harp.device.treadmill](https://github.com/AllenNeuralDynamics/harp.device.treadmill)
* [harp.device.ttl-io](https://github.com/AllenNeuralDynamics/harp.device.ttl-io)

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
`printf` messages are sprinkled throughout the Harp Core code, and they can be conditionally compiled in if you add the following to your CMakeLists.txt:
````cmake
add_definitions(-DDEBUG)
````

# References
* [Harp Protocol Repo](https://github.com/harp-tech/protocol)
* [pyharp](https://github.com/harp-tech/pyharp) python library for connecting to harp-compliant devices and sending read/writes.
