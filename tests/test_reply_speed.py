#!/usr/bin/env python3
from pyharp.device import Device, DeviceMode
from pyharp.messages import HarpMessage
from pyharp.messages import MessageType
from pyharp.messages import CommonRegisters as Regs
from struct import *
import numpy as np
import os
from time import sleep, perf_counter
from matplotlib import pyplot as plt


ROUND_TRIPS = 50000


# Open the device and print the info on screen
# Open serial connection and save communication to a file
if os.name == 'posix': # check for Linux.
    #device = Device("/dev/harp_device_00", "ibl.bin")
    device = Device("/dev/ttyACM0", "ibl.bin")
else: # assume Windows.
    device = Device("COM95", "ibl.bin")


timestamps_t = np.zeros(ROUND_TRIPS, dtype=float);

print(f"Performing {ROUND_TRIPS}x round trips. "
       "(Message from PC to Harp device. Reply from Harp device to PC.)")
for i in range(ROUND_TRIPS):
    #timestamps_t[i] = device.send(HarpMessage.ReadU8(Regs.OPERATION_CTRL).frame).timestamp
    device.send(HarpMessage.ReadU8(Regs.OPERATION_CTRL).frame).timestamp
    timestamps_t[i] = perf_counter()

time_deltas_t = np.diff(timestamps_t)
print(f"Summary:")
print(f"mean: {np.mean(time_deltas_t):.6f}")
print(f"std dev: {np.std(time_deltas_t):.6f}")
print(f"max: {np.max(time_deltas_t):.6f} at index: {np.argmax(time_deltas_t)}")
#print(time_deltas_t[argmax - 3: argmax + 3])
#print(timestamps_t[argmax - 3: argmax + 3])
print()
large_value_locations = np.where(time_deltas_t > 0.003)
print("The following delay times are large:")
print(large_value_locations)
print("indexes of the large delay times:")
print([time_deltas_t[i] for i in large_value_locations])


# Close connection
device.disconnect()

plt.hist(time_deltas_t, bins='auto')
plt.show()
