#ifndef HARP_CORE_H
#define HARP_CORE_H
#include <stdint.h>
#include <harp_message.h>
#include <core_registers.h>
#include <arm_regs.h>
#include <functional> // for std::invoke
#include <cstring> // for memcpy
#include <tusb.h>

// Pico-specific includes.
#include <hardware/structs/timer.h>
#include <hardware/timer.h>

// Create a typedef to simplify syntax for array of static function ptrs.
typedef void (*read_reg_fn)(uint8_t reg);
typedef void (*write_reg_fn)(msg_t& msg);

// Convenience struct for aggregating an array of fn ptrs to handle each
// register.
// TODO: this struct has the same contents as RegFnPair?? If so, consolidate.
struct RegFnPair
{
    read_reg_fn read_fn_ptr;
    write_reg_fn  write_fn_ptr;
};

/**
 * \brief Harp Core that handles management of common bank registers.
*       Implemented as a singleton to simplify attaching interrupt callbacks
*       (and since you can only have one per device.
 */
class HarpCore
{
// Make constructor protected to prevent creating instances outside of init().
protected: // protected, but not private, to enable derived class usage.
    HarpCore(uint16_t who_am_i,
             uint8_t hw_version_major, uint8_t hw_version_minor,
             uint8_t assembly_version,
             uint8_t harp_version_major, uint8_t harp_version_minor,
             uint8_t fw_version_major, uint8_t fw_version_minor,
             uint16_t serial_number, const char name[]);

    ~HarpCore();

public:
    HarpCore() = delete;  // Disable default constructor.
    HarpCore(HarpCore& other) = delete; // Disable copy constructor.
    void operator=(const HarpCore& other) = delete; // Disable assignment operator.

/**
 * \brief initialize the harp core singleton with parameters and init Tinyusb.
 */
    static HarpCore& init(uint16_t who_am_i,
                          uint8_t hw_version_major, uint8_t hw_version_minor,
                          uint8_t assembly_version,
                          uint8_t harp_version_major, uint8_t harp_version_minor,
                          uint8_t fw_version_major, uint8_t fw_version_minor,
                          uint16_t serial_number, const char name[]);

    static inline HarpCore* self = nullptr;
    static HarpCore& instance() {return *self;}


/**
 * \brief Periodically handle tasks based on the current time, state,
 *      and inputs. Should be called in a loop.
 */
    void run();

/**
 * \brief Read incoming bytes from the USB serial port. Does not block.
 *  \note If called again after returning true, the buffered message may be
 *      be overwritten.
 */
    void process_cdc_input();

/**
 * \brief return a reference to the message header in the rx_buffer.
 * \note this should only be accessed if process_cdc_input returns true or
 *      total_bytes_read_ >= sizeof(msg_header_t).
 */
    msg_header_t& get_buffered_msg_header()
    {return *((msg_header_t*)(&rx_buffer_));}

/**
 * \brief return a reference to the message in the rx_buffer. Inline.
 * \note this should only be accessed if process_cdc_input returns true or
 *      total_bytes_read_ >= sizeof(msg_header_t).
 */
    msg_t get_buffered_msg();


/**
 * \brief Write register contents to the tx buffer by dispatching message
 *      to the appropriate handler. Inline.
 */
//    void read_from_reg(uint8_t reg)
//    {reg_func_table_[reg].read_fn_ptr(reg);}

/**
 * \brief Write message contents to a register by dispatching message
 *      to the appropriate handler. Inline.
 */
//    void write_to_reg(msg_t& msg)
//    {reg_func_table_[msg.header.address].write_fn_ptr(msg);}

/**
 * \brief reference to the struct of reg values for easy access.
 */
    RegValues& regs = regs_.regs_;

/**
 * \brief the total number of bytes read into the the msg receive buffer.
 *  This is implemented as a read-only reference to the rx_buffer_index_.
 */
    const uint8_t& total_bytes_read_;


/**
 * \brief flag indicating whether or not a new message is in the rx_buffer_.
 */
    bool new_msg()
    {return new_msg_;}

/**
 * \brief flag that new message has been handled. Inline.
 * \note Does not affect internal behavior.
 */
    void clear_msg()
    {new_msg_ = false;}

/**
 * \brief generic handler function to write a message payload to a core or
 *      app register and issue a harp reply (unless muted).
 * \note this function may be used in cases where no actions must trigger from
        writing to this register.
 * \note since the struct is byte-aligned, writing more data than the size of
 *      the register will sequentially write to the next register within the
 *      app range and core range. In this way,
 *      you can write to multiple sequential registers starting from the
 *      msg.address.
 */
    static void write_reg_generic(msg_t& msg);

/**
 * \brief generic handler function to read a message payload to a core or
 *      app register and issue a harp reply (unless muted).
 * \note this function may be used in cases where (1) the register value is
 *      up-to-date and (2) no actions must trigger from reading this register.
 */
    static void read_reg_generic(uint8_t reg_name);

/**
 * \brief write handler function. Sends a harp reply indicating a write error
 *      to the specified register.
 */
    static void write_to_read_only_reg_error(msg_t& msg);

protected:
/**
 * \brief entry point for handling incoming harp messages to core registers.
 *      Dispatches message to the appropriate handler.
 */
    void handle_buffered_core_message();

/**
 * \brief Handle incoming messages for the derived class. Does nothing here,
 *  but not pure virtual since we need to be able to instantiate a standalone
 *  harp core.
 */
    virtual void handle_buffered_app_message(){};

/**
 * \brief update internal state machine.
 */
    void update_state();

/**
 * \brief update state of the derived class. Does nothing here,
 *  but not pure virtual since we need to be able to instantiate a standalone
 *  harp core.
 */
    virtual void update_app_state(){};


    virtual const RegSpecs& address_to_app_reg_specs(uint8_t address)
    {return regs_.enum_to_reg_specs[0];} // should never happen.

/**
 * \brief Send a Harp-compliant timestamped reply message.
 * \warning does not check if we are currently busy sending a harp reply.
 * \note made static such that we can write functions that invoke it before
 *  instantiating the HarpCore singleton.
 */
    static void send_harp_reply(msg_type_t reply_type, uint8_t reg_name,
                                const volatile uint8_t* data, uint8_t num_bytes,
                                reg_type_t payload_type);

/**
 * \brief true if the mute flag has been set in the R_OPERATION_CTRL register.
 */
    bool is_muted()
    {return bool((regs.R_OPERATION_CTRL >> MUTE_RPL_OFFSET) & 0x01);}

    bool events_enabled()
    {return (regs.R_OPERATION_CTRL & 0x03) == ACTIVE;}

/**
 * \brief data is read from serial port into the the rx_buffer.
 */
    uint8_t rx_buffer_[MAX_PACKET_SIZE];
    uint8_t rx_buffer_index_;
/**
 * \brief flag indicating whether or not a new message is in the rx_buffer_.
 */
    bool new_msg_;

private:
    void update_timestamp_regs();  // call before reading timestamp register.

    const RegSpecs& reg_address_to_specs(uint8_t address);

/**
 * \brief read handler functions. One-per-harp-register where necessary,
 *      but the generic one can be used in most cases.
 *      Note: these all need to have the same function signature.
 */
    static void read_timestamp_second(uint8_t reg_name);
    static void read_timestamp_microsecond(uint8_t reg_name);
    // See also read_reg_generic

/**
 * \brief a write handler function per harp register. Handles write
 *      operations to that register.
 */
    static void write_timestamp_second(msg_t& msg);
    static void write_timestamp_microsecond(msg_t& msg);
    static void write_operation_ctrl(msg_t& msg);
    static void write_reset_def(msg_t& msg);
    static void write_device_name(msg_t& msg);
    static void write_serial_number(msg_t& msg);
    static void write_clock_config(msg_t& msg);
    static void write_timestamp_offset(msg_t& msg);
    // See also write_reg_generic




/**
 *  \brief Struct of registers.
 */
    Registers regs_;

/**
 * \brief Function Table. Order matters since we will index into it with enums.
 */
    RegFnPair reg_func_table_[CORE_REG_COUNT] =
    {
        // { <read_fn_ptr>, <write_fn_prt>},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_reg_generic, &HarpCore::write_to_read_only_reg_error},
        {&HarpCore::read_timestamp_second, &HarpCore::write_timestamp_second},
        {&HarpCore::read_timestamp_microsecond, &HarpCore::write_timestamp_microsecond},
        {&HarpCore::read_reg_generic, &HarpCore::write_operation_ctrl},
        {&HarpCore::read_reg_generic, &HarpCore::write_reset_def},
        {&HarpCore::read_reg_generic, &HarpCore::write_device_name},
        {&HarpCore::read_reg_generic, &HarpCore::write_serial_number},
        {&HarpCore::read_reg_generic, &HarpCore::write_clock_config},
        {&HarpCore::read_reg_generic, &HarpCore::write_timestamp_offset}
    };
};

#endif //HARP_CORE_H
