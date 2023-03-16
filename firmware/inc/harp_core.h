#ifndef HARP_CORE_H
#define HARP_CORE_H
#include <stdint.h>
#include <harp_message.h>
#include <core_registers.h>
#include <arm_regs.h>
#include <functional>  // for std::invoke

#include <hardware/structs/timer.h>
#include <hardware/timer.h>
#include <tusb.h>


/**
 * \brief Harp Core that handles management of common bank registers.
*       Implemented as a singleton to simplify attaching interrupt callbacks
*       (and since you can only have one per device.
 */
class HarpCore
{
// Make constructor private to prevent creating instances outside of init().
private:
    HarpCore(uint16_t who_am_i,
             uint8_t hw_version_major, uint8_t hw_version_minor,
             uint8_t assembly_version,
             uint8_t harp_version_major, uint8_t harp_version_minor,
             uint8_t fw_version_major, uint8_t fw_version_minor,
             const char name[]);

    ~HarpCore();

public:
    HarpCore() = delete;  // Disable default constructor.
    HarpCore(HarpCore& other) = delete; // Disable copy constructor.
    void operator=(const HarpCore& other) = delete; // Disable assignment operator.

/**
 * \brief initialize the harp core singleton with parameters.
 */
    static HarpCore& init(uint16_t who_am_i,
                          uint8_t hw_version_major, uint8_t hw_version_minor,
                          uint8_t assembly_version,
                          uint8_t harp_version_major, uint8_t harp_version_minor,
                          uint8_t fw_version_major, uint8_t fw_version_minor,
                          const char name[]);

    static inline HarpCore* self = nullptr;
    static HarpCore& instance() {return *self;}

    // Create a typedef so we can create an array of member function ptrs.
    // MsgHandleMemberFn points to a harp core member fn that takes (msg_t&)
    // https://isocpp.org/wiki/faq/pointers-to-members#array-memfnptrs
    typedef void (HarpCore::*RegReadMemberFn)(RegName reg);
    typedef void (HarpCore::*RegWriteMemberFn)(msg_t& mst);

/**
 * \brief Periodically handle tasks based on the current time, state,
 *      and inputs. Should be called in a loop.
 */
    void run();

/**
 * \brief Read incoming bytes from the USB serial port. Calls
 *      handle_rx_buffer_message. Does not block waiting for incoming chars.
 */
    void handle_rx_buffer_input();

/**
 * \brief entry point for handling incoming harp messages. Dispatches message
 *      to the appropriate handler.
 */
    void handle_rx_buffer_message();

/**
 * \brief update state machine handling register logic.
 */
    void update_register_state_outputs();

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
    void read_from_reg(RegName reg)
    {std::invoke(reg_read_fns_[reg], this, reg);}


/**
 * \brief data is read from serial port into the the rx_buffer.
 */
    uint8_t rx_buffer_[MAX_PACKET_SIZE];  // TODO: should be private.
    uint8_t rx_buffer_index_;

/**
 * \brief reference to the struct of reg values for easy access.
 */
    RegValues& regs = regs_.regs_;

protected:
/**
 * \brief Send a Harp-compliant timestamped reply message.
 * \warning does not check if we are currently busy sending a harp reply.
 */
    void send_harp_reply(msg_type_t reply_type, RegName reg_name,
                         const volatile uint8_t* data, uint8_t num_bytes,
                         reg_type_t payload_type);

    bool is_muted()
    {return bool((regs.R_OPERATION_CTRL >> MUTE_RPL_OFFSET) & 0x01);}

    bool events_enabled()
    {return (regs.R_OPERATION_CTRL & 0x03) == ACTIVE;}

private:

    void update_timestamp_regs();  // call before reading timestamp register.

/**
 * \brief read handler functions. One-per-harp-register where necessary,
 *      but the generic one can be used in most cases.
 *      Note: these all need to have the same function signature.
 */
    void read_timestamp_second(RegName reg_name);
    void read_timestamp_microsecond(RegName reg_name);
    void read_reg_generic(RegName reg_name); // catch-all.

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

/**
 *  \brief registers.
 */
    Registers regs_;

    // Function Tables. Order matters since we will index into it with enums.
    RegReadMemberFn reg_read_fns_[CORE_REG_COUNT] =
    {
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_timestamp_second,
        &HarpCore::read_timestamp_microsecond,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
        &HarpCore::read_reg_generic,
    };

    RegWriteMemberFn reg_write_fns_[CORE_REG_COUNT] =
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
