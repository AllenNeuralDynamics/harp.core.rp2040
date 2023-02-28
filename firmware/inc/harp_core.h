#ifndef HARP_CORE_H
#define HARP_CORE_H
#include <stdint.h>
#include <harp_message.h>
#include <registers.h>
#include <arm_regs.h>
#include <functional>  // for std::invoke

#include <pico/stdlib.h> // TODO: remove this later.


class HarpCore
{
public:
        HarpCore(uint16_t who_am_i, uint16_t hw_version,
                  uint8_t assembly_version, uint16_t harp_version,
                  uint16_t fw_version);
        ~HarpCore();

    // Create a typedef so we can create an array of member function ptrs.
    // MsgHandleMemberFn points to a harp core member fn that takes (msg_t&)
    // https://isocpp.org/wiki/faq/pointers-to-members#array-memfnptrs
    typedef void (HarpCore::*RegReadMemberFn)(RegNames reg);
    typedef void (HarpCore::*RegWriteMemberFn)(msg_t& mst);

/**
 * \brief parse the serial data into message fields. Return a msg_header_t.
 *  inline.
 */
// UNUSED
//    msg_header_t& get_msg_header_from_buffer()
//    {return *((msg_header_t*)(&rx_buffer_));}

/**
 * \brief entry point for handling incoming harp messages. Dispatches message
 *      to the appropriate handler.
 */
    void handle_rx_buffer_message();

/**
 * \brief Write message contents to a register by dispatching message
 *      to the appropriate handler. inline.
 */
    void write_to_reg(msg_t& msg)
    {std::invoke(reg_write_fns_[msg.header.address], this, msg);}

/**
 * \brief Write register contents to the tx buffer by dispatching message
 *      to the appropriate handler. inline.
 */
    void read_from_reg(RegNames reg)
    {std::invoke(reg_read_fns_[reg], this, reg);}

/**
 *  \brief registers.
 */
    Registers regs_;  // TODO: should be private.
/**
 * \brief data is read from serial port into the the rx_buffer.
 */
    uint8_t rx_buffer_[MAX_PACKET_SIZE];  // TODO: should be private.

private:

/**
 * \brief read handler functions. One-per-harp-register where necessary,
 *      but the generic one can be used in most cases.
 */
    void read_timestamp_microsecond(RegNames reg_name);
    void read_reg_generic(RegNames reg_name); // catch-all.

/**
 * \brief a write handler function per harp register. Handles write
 *      operations to that register.
 */
    void write_timestamp_second(msg_t& msg);
    void write_timestamp_microsecond(msg_t& msg);
    void write_operation_ctrl(msg_t& msg);
    void write_reset_def(msg_t& msg);
    void write_device_name(msg_t& msg);
    void write_serial_number(msg_t& msg);
    void write_clock_config(msg_t& msg);
    void write_timestamp_offset(msg_t& msg);

    void write_to_read_only_reg_error(msg_t& msg);



    // Function Tables. Order matters since we will index into it with enums.
    RegReadMemberFn reg_read_fns_[REG_COUNT] =
    {
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_timestamp_microsecond,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
    };

    RegWriteMemberFn reg_write_fns_[REG_COUNT] =
    {&HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_to_read_only_reg_error,
     &HarpCore::write_timestamp_second,
     &HarpCore::write_timestamp_microsecond,
     &HarpCore::write_operation_ctrl,
     &HarpCore::write_reset_def,
     &HarpCore::write_device_name,
     &HarpCore::write_serial_number,
     &HarpCore::write_clock_config,
     &HarpCore::write_timestamp_offset,
    };
};

#endif //HARP_CORE_H
