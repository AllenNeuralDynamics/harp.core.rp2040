#ifndef REGISTERS_H
#define REGISTERS_H
#include <stdint.h>
#include <reg_types.h>
#include <core_reg_bits.h>
#include <cstring>  // for strcpy

static const uint8_t REG_COUNT = 16;

// R_OPERATION_CTRL bitfields.
#define DUMP_OFFSET (3)
#define MUTE_RPL_OFFSET (4)
#define VISUAL_EN_OFFSET (5)
#define OPLEDEN (6)
#define ALIVE_EN (7)

/**
 * \brief enum for easier interpretation of the OP_MODE bitfield in the
 *  R_OPERATION_CTRL register.
 */
enum op_mode_t: uint8_t
{
    STANDBY = 0,
    ACTIVE = 1,
    RESERVED = 2,
    SPEED = 3
};


/**
 * \brief enum where the name is the name of the register and the
 *        value is the address according to the harp protocol spec.
 */
enum RegNames : uint8_t  // FIXME: make RegName
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


// Byte-align struct data so we can send it out serially byte-by-byte.
#pragma pack(push, 1)
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
#pragma pack(pop)

struct RegSpecs
{
    const volatile uint8_t* base_ptr;
    const uint8_t num_bytes;
    const reg_type_t payload_type;
};

struct Registers
{
    public:
        Registers(uint16_t who_am_i,
                  uint8_t hw_version_major, uint8_t hw_version_minor,
                  uint8_t assembly_version,
                  uint8_t harp_version_major, uint8_t harp_version_minor,
                  uint8_t fw_version_major, uint8_t fw_version_minor,
                  const char name[]);
        ~Registers();

    RegValues regs_;
//    RegValues& regs = regs_;

    // Lookup table. Necessary because register data is not of equal size,
    //  so we can't index into it directly by enum.
    // TODO: consider generating this table statically with a template.
    const RegSpecs enum_to_reg_specs[REG_COUNT] =
    {{(uint8_t*)&regs_.R_WHO_AM_I,         sizeof(regs_.R_WHO_AM_I),          U16},
     {(uint8_t*)&regs_.R_HW_VERSION_H,     sizeof(regs_.R_HW_VERSION_H),      U8},
     {(uint8_t*)&regs_.R_HW_VERSION_L,     sizeof(regs_.R_HW_VERSION_L),      U8},
     {(uint8_t*)&regs_.R_ASSEMBLY_VERSION, sizeof(regs_.R_ASSEMBLY_VERSION),  U8},
     {(uint8_t*)&regs_.R_HARP_VERSION_H,   sizeof(regs_.R_HARP_VERSION_H),    U8},
     {(uint8_t*)&regs_.R_HARP_VERSION_L,   sizeof(regs_.R_HW_VERSION_L),      U8},
     {(uint8_t*)&regs_.R_FW_VERSION_H,     sizeof(regs_.R_FW_VERSION_H),      U8},
     {(uint8_t*)&regs_.R_FW_VERSION_L,     sizeof(regs_.R_FW_VERSION_L),      U8},
     {(uint8_t*)&regs_.R_TIMESTAMP_SECOND, sizeof(regs_.R_TIMESTAMP_SECOND),  U32},
     {(uint8_t*)&regs_.R_TIMESTAMP_MICRO,  sizeof(regs_.R_TIMESTAMP_MICRO),   U16},
     {(uint8_t*)&regs_.R_OPERATION_CTRL,   sizeof(regs_.R_OPERATION_CTRL),    U8},
     {(uint8_t*)&regs_.R_RESET_DEF,        sizeof(regs_.R_RESET_DEF),         U8},
     {(uint8_t*)&regs_.R_DEVICE_NAME,      sizeof(regs_.R_DEVICE_NAME),       U8},
     {(uint8_t*)&regs_.R_SERIAL_NUMBER,    sizeof(regs_.R_SERIAL_NUMBER),     U16},
     {(uint8_t*)&regs_.R_CLOCK_CONFIG,     sizeof(regs_.R_CLOCK_CONFIG),      U8},
     {(uint8_t*)&regs_.R_TIMESTAMP_OFFSET, sizeof(regs_.R_TIMESTAMP_OFFSET),  U8}};

    // Syntactic Sugar. Make bitfields for certain registers easier to access.
    OperationCtrlBits& r_operation_ctrl_bits = *((OperationCtrlBits*)(&regs_.R_OPERATION_CTRL));
    ResetDefBits& r_reset_def_bits = *((ResetDefBits*)(&regs_.R_RESET_DEF));
    ClockConfigBits& r_clock_config_bits = *((ClockConfigBits*)(&regs_.R_CLOCK_CONFIG));
};

#endif //REGISTERS_H
