#!/usr/bin/env python3
from pyharp.device import Device, DeviceMode
from pyharp.messages import HarpMessage
from pyharp.messages import MessageType
from pyharp.messages import CommonRegisters as Regs
from struct import *
import os


# ON THIS EXAMPLE
#
# This code opens the connection with the device and displays the information
# Also saves device's information into variables


# Open the device and print the info on screen
# Open serial connection and save communication to a file
if os.name == 'posix': # check for Linux.
    #device = Device("/dev/harp_device_00", "ibl.bin")
    device = Device("/dev/ttyACM0", "ibl.bin")
else: # assume Windows.
    device = Device("COM95", "ibl.bin")
device.info()                           # Display device's info on screen
# dump registers.
print("Register dump:")
print(device.dump_registers())
# Close connection
device.disconnect()
