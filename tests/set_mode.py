#!/usr/bin/env python3
from pyharp.device import Device, DeviceMode
from pyharp.messages import HarpMessage
from pyharp.messages import MessageType
from pyharp.messages import CommonRegisters as Regs
from struct import *
import os
from time import sleep, perf_counter


# Open the device and print the info on screen
# Open serial connection and save communication to a file
if os.name == 'posix': # check for Linux.
    #device = Device("/dev/harp_device_00", "ibl.bin")
    device = Device("/dev/ttyACM0", "ibl.bin")
else: # assume Windows.
    device = Device("COM95", "ibl.bin")


for i in range(2):
    print("Setting device mode to Active.")
    reply = device.set_mode(DeviceMode.Active)
    print("reply to set mode is: ")
    print(reply)
    print(f"Device mode is now: {device.read_device_mode()}")

    print("Setting device mode to Standby.")
    reply = device.set_mode(DeviceMode.Standby)
    print("reply to set mode is: ")
    print(reply)
    print(f"Device mode is now: {device.read_device_mode()}")

    print()
    sleep(0.5)

# Close connection
device.disconnect()
