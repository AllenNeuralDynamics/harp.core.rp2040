## Setting up the Build Environment
This project uses the Raspberry Pi Pico SDK as a submodule (which itself contains submodules).
Clone and setup all required submodules with:
````
git submodule update --init --recursive
````

## Compiling the Firmware

### Without an IDE
From this directory, create a directory called build, enter it, and invoke cmake with:
````
mkdir build
cd build
cmake ..
````
After this point, you can invoke the auto-generated Makefile with `make`

## Flashing the Firmware
Press-and-hold the Pico's BOOTSEL button and power it up (i.e: plug it into usb).
At this point you do one of the following:
* drag-and-drop the created **\*.uf2** file into the mass storage device that appears on your pc.
* flash with [picotool](https://github.com/raspberrypi/picotool)


## Editing the Firmware
If you edit the CMakeLists.txt, you need to update the build folder (or recreate it).
If the build folder already exists, from within it invoke `cmake ..` like above.
Then invoke `make` as usual.

## Implementation Details
TODO
