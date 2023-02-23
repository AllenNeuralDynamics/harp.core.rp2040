#ifndef REGISTERS_H
#define REGISTERS_H
#include <stdint.h>

static const uint8_t REG_COUNT = 16;

/**
 * \brief enum where the name is the name of the register and the
 *        value is the address according to the harp protocol spec.
 */
enum RegNames
{
    WHO_AM_I = 0,
    HW_VERSION_H = 1, // major hardware version
    HW_VERSION_L = 2, // minor hardware version
    ASSEMBLY_VERSION = 3,
    HARP_VERSION_H = 4,
    HARP_VERSION_L = 5,
    FW_VERSION_H = 6,
    FW_VERSION_L = 7,
    TIMESTAMP_SECOND = 8,
    TIMESTAMP_MICRO = 9,
    OPERATION_CTRL = 10,
    RESET_DEF = 11,
    DEVICE_NAME = 12,
    SERIAL_NUMBER = 13,
    CLOCK_CONFIG = 14,
    TIMESTAMP_OFFSET = 15
};


struct RegValues
{
    const uint16_t R_WHO_AM_I;
    const uint8_t R_HW_VERSION_H;
    const uint8_t R_HW_VERSION_L;
    const uint8_t R_ASSEMBLY_VERSION;
    const uint8_t R_HARP_VERSION_H;
    const uint8_t R_HARP_VERSION_L ;
    const uint8_t R_FW_VERSION_H;
    const uint8_t R_FW_VERSION_L;
    volatile uint32_t R_TIMESTAMP_SECOND;
    volatile uint16_t R_TIMESTAMP_MICRO;
    volatile uint8_t R_OPERATION_CTRL;
    volatile uint8_t R_RESET_DEF;
    volatile char R_DEVICE_NAME[25];
    volatile uint16_t R_SERIAL_NUMBER;
    volatile uint8_t R_CLOCK_CONFIG;
    volatile uint8_t R_TIMESTAMP_OFFSET;
};

struct Registers
{
    public:
        Registers(uint16_t who_am_i, uint16_t hw_version,
                  uint8_t assembly_version, uint16_t harp_version,
                  uint16_t fw_version);
        ~Registers();

    RegValues regs_;
    RegValues& regs = regs_;
    // Lookup table. Might not be needed.
    const volatile void* name2reg[REG_COUNT] =
        {&regs_.R_WHO_AM_I,
         &regs_.R_HW_VERSION_H,
         &regs_.R_HW_VERSION_L,
         &regs_.R_ASSEMBLY_VERSION,
         &regs_.R_HARP_VERSION_H,
         &regs_.R_HARP_VERSION_L,
         &regs_.R_FW_VERSION_H,
         &regs_.R_FW_VERSION_L,
         &regs_.R_TIMESTAMP_SECOND,
         &regs_.R_TIMESTAMP_MICRO,
         &regs_.R_OPERATION_CTRL,
         &regs_.R_RESET_DEF,
         &regs_.R_DEVICE_NAME,
         &regs_.R_SERIAL_NUMBER,
         &regs_.R_CLOCK_CONFIG,
         &regs_.R_TIMESTAMP_OFFSET};
};

#endif //REGISTERS_H
