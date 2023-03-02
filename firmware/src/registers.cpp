#include <registers.h>

Registers::Registers(uint16_t who_am_i, uint16_t hw_version,
                     uint8_t assembly_version, uint16_t harp_version,
                     uint16_t fw_version, const char name[])
:regs_{.R_WHO_AM_I = who_am_i,
       .R_HW_VERSION_H = uint8_t(hw_version>>8),
       .R_HW_VERSION_L = uint8_t(hw_version&0x00FF),
       .R_ASSEMBLY_VERSION = assembly_version,
       .R_HARP_VERSION_H = uint8_t(harp_version>>8),
       .R_HARP_VERSION_L = uint8_t(harp_version&0x00FF),
       .R_FW_VERSION_H = uint8_t(fw_version>>8),
       .R_FW_VERSION_L = uint8_t(fw_version&0x00FF)}
{
    strcpy((char*)regs_.R_DEVICE_NAME, name);
}


Registers::~Registers(){}
