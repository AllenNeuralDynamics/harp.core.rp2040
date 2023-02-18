#ifndef REGISTERS_H
#define REGISTERS_H
#include <stdint.h>
#include <stdalign.h>

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

struct Registers
{
    alignas(uint32_t) const uint16_t R_WHO_AM_I;
    alignas(uint32_t) const uint8_t R_HW_VERSION_H;
    alignas(uint32_t) const uint8_t R_HW_VERSION_L;
    alignas(uint32_t) const uint8_t R_ASSEMBLY_VERSION;
    alignas(uint32_t) const uint8_t R_HARP_VERSION_H;
    alignas(uint32_t) const uint8_t R_HARP_VERSION_L ;
    alignas(uint32_t) const uint8_t R_FW_VERSION_H;
    alignas(uint32_t) const uint8_t R_FW_VERSION_L;
    alignas(uint32_t) volatile uint32_t R_TIMESTAMP_SECOND;
    alignas(uint32_t) volatile uint16_t R_TIMESTAMP_MICRO;
    alignas(uint32_t) volatile uint8_t R_OPERATION_CTRL;
    alignas(uint32_t) volatile uint8_t R_RESET_DEF;
    alignas(uint32_t) volatile uint8_t R_DEVICE_NAME;
    alignas(uint32_t) volatile uint16_t R_SERIAL_NUMBER;
    alignas(uint32_t) volatile uint8_t R_CLOCK_CONFIG;
    alignas(uint32_t) volatile uint8_t R_TIMESTAMP_OFFSET;
};

/**
 * \brief struct representing reg data in contiguous 32-bit word-aligned memory.
 *      Data can be accessed via name or protocol address.
 *
 * \note Storing the data in contiguous 32-bit word-aligned memory creates a
 *       larger memory footprint, but prevents the need to create a lookup
 *       table from name to memory location, which itself would also have a
 *       substantial footprint.
 *
 * \code{.cpp}
 * RegMemory reg_mem = RegMemory{0x0000, 0x00, 0x00, 0x00,
                                 0x00, 0x00, 0x00, 0x00,
                                 0x00000000, 0x0000, 0x00, 0x00,
                                 0x00, 0x0000, 0x00, 0x00} ;
*
* // Conventional struct access:
* printf("Device ID: %d", reg_mem.names.R_WHO_AM_I)
* printf("Timestamp sec: %d", reg_mem.names.R_TIMESTAMP_SECOND)
*
* // Access by protocol address.
* printf"Device ID: %d", reg_mem.mem[RegNames::WHO_AM_I])
 * \endcode
 */
union RegMemory
{
    Registers names;
    uint32_t mem[sizeof(names)/sizeof(uint32_t)];
};
#endif //REGISTERS_H
