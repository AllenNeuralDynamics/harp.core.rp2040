#include <core_registers.h>

Registers::Registers(uint16_t who_am_i,
                     uint8_t hw_version_major, uint8_t hw_version_minor,
                     uint8_t assembly_version,
                     uint8_t harp_version_major, uint8_t harp_version_minor,
                     uint8_t fw_version_major, uint8_t fw_version_minor,
                     uint16_t serial_number, const char name[],
                     const uint8_t tag[])
:regs_{.R_WHO_AM_I = who_am_i,
       .R_HW_VERSION_H = hw_version_major,
       .R_HW_VERSION_L = hw_version_minor,
       .R_ASSEMBLY_VERSION = assembly_version,
       .R_HARP_VERSION_H = harp_version_major,
       .R_HARP_VERSION_L = harp_version_minor,
       .R_FW_VERSION_H = fw_version_major,
       .R_FW_VERSION_L = fw_version_minor,
       .R_OPERATION_CTRL = 0,
       .R_SERIAL_NUMBER = serial_number,
       .R_UUID = {0} // all zeros.
        }
{
    strcpy((char*)regs_.R_DEVICE_NAME, name);
    strcpy((char*)regs_.R_TAG, (char*)tag);
}


Registers::~Registers(){}
