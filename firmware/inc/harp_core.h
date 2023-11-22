#ifndef HARP_CORE_H
#define HARP_CORE_H
#include <stdint.h>
#include <harp_message.h>
#include <core_registers.h>
#include <arm_regs.h>
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
struct RegFnPair
{
    read_reg_fn read_fn_ptr;
    write_reg_fn  write_fn_ptr;
};

/**
 * \brief Harp Core that handles management of common bank registers.
*       Implemented as a singleton to simplify attaching interrupt callbacks
*       (and since you can only have one per device.)
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
 * \note default constructor, copy constructor, and assignment operator have
 *  been disabled.
 */
    static HarpCore& init(uint16_t who_am_i,
                          uint8_t hw_version_major, uint8_t hw_version_minor,
                          uint8_t assembly_version,
                          uint8_t harp_version_major, uint8_t harp_version_minor,
                          uint8_t fw_version_major, uint8_t fw_version_minor,
                          uint16_t serial_number, const char name[]);

    static inline HarpCore* self = nullptr; // pointer to the singleton instance.
    static HarpCore& instance() {return *self;} ///< returns the singleton.


/**
 * \brief Periodically handle tasks based on the current time, state,
 *      and inputs. Should be called in a loop. Calls tud_task() and
 *      process_cdc_input().
 */
    void run();

/**
 * \brief return a reference to the message header in the #rx_buffer_.
 * \warning this should only be accessed if new_msg() is true.
 */
    msg_header_t& get_buffered_msg_header()
    {return *((msg_header_t*)(&rx_buffer_));}

/**
 * \brief return a reference to the message in the #rx_buffer_. Inline.
 * \warning this should only be accessed if new_msg() is true.
 */
    msg_t get_buffered_msg();

/**
 * \brief reference to the struct of reg values for easy access.
 */
    RegValues& regs = regs_.regs_;

/**
 * \brief flag indicating whether or not a new message is in the #rx_buffer_.
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
 *      app register and issue a harp reply (unless is_muted()).
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
 *      app register and issue a harp reply (unless is_muted()).
 * \note this function may be used in cases where (1) the register value is
 *      up-to-date and (2) no actions must trigger from reading this register.
 */
    static void read_reg_generic(uint8_t reg_name);

/**
 * \brief write handler function. Sends a harp reply indicating a write error
 *      to the specified register.
 */
    static void write_to_read_only_reg_error(msg_t& msg);

/**
 * \brief update local register data with the payload provided in the input msg.
 */
    static inline void copy_msg_payload_to_register(msg_t& msg)
    {
        const RegSpecs& specs = self->reg_address_to_specs(msg.header.address);
        memcpy((void*)specs.base_ptr, msg.payload, specs.num_bytes);
    }

/**
 * \brief Send a Harp-compliant timestamped reply message for the specified
 *  core or app register.
 * \details this function will lookup the particular core-or-app register's
 *  specs for the provided address and construct a reply based on those specs.
 * \note this function is static such that we can write functions that invoke it
 *  before instantiating the HarpCore singleton.
 * \note Calls tud_task().
 */
    static inline void send_harp_reply(msg_type_t reply_type, uint8_t reg_name)
    {
        const RegSpecs& specs = self->reg_address_to_specs(reg_name);
        send_harp_reply(reply_type, reg_name, specs.base_ptr, specs.num_bytes,
                        specs.payload_type);
    }

/**
 * \brief Construct and send a Harp-compliant timestamped reply message from
 *  provided arguments.
 * \note this function is static such that we can write functions that invoke it
 *  before instantiating the HarpCore singleton.
 * \note Calls tud_task().
 */
    static void send_harp_reply(msg_type_t reply_type, uint8_t reg_name,
                                const volatile uint8_t* data, uint8_t num_bytes,
                                reg_type_t payload_type);

/**
 * \brief true if the mute flag has been set in the R_OPERATION_CTRL register.
 */
    static bool is_muted()
    {return bool((self->regs.R_OPERATION_CTRL >> MUTE_RPL_OFFSET) & 0x01);}

    static bool events_enabled()
    {return (self->regs.R_OPERATION_CTRL & 0x03) == ACTIVE;}

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
 * \brief update state of the derived class. Does nothing in the base class,
 *  but not pure virtual since we need to be able to instantiate a standalone
 *  harp core.
 */
    virtual void update_app_state(){};

/**
 * \brief reset the app. Called when the writing to the RESET_DEF register.
 *  Does nothing in the base class, but not pure virtual since we need to be
 *  able to instantiate a standalone harp core.
 */
    virtual void reset_app(){};

/**
 * \brief Enable or disable external virtual indicators.
 *  Does nothing in the base class, but not pure virtual since we need to be
 *  able to instantiate a standalone harp core.
 */
    virtual void set_visual_indicators(bool enabled){};

/**
 * \brief send one harp reply read message per app register.
 *  Called when the writing to the R_OPERATION_CTRL's DUMP bit.
 *  Does nothing in the base class, but not pure virtual since we need to be
 *  able to instantiate a standalone harp core.
 */
    virtual void dump_app_registers(){};

    virtual const RegSpecs& address_to_app_reg_specs(uint8_t address)
    {return regs_.address_to_specs[0];} // should never happen.


/**
 * \brief flag indicating whether or not a new message is in the rx_buffer_.
 */
    bool new_msg_;

private:
/**
 * \brief the total number of bytes read into the the msg receive buffer.
 *  This is implemented as a read-only reference to the #rx_buffer_index_.
 */
    const uint8_t& total_bytes_read_;

/**
 * \brief buffer to contain data read from the serial port.
 */
    uint8_t rx_buffer_[MAX_PACKET_SIZE];

/**
 * \brief #rx_buffer_ index where the next incoming byte will be written.
 */
    uint8_t rx_buffer_index_;

/**
 * \brief Read incoming bytes from the USB serial port. Does not block.
 *  \warning If called again before handling previous message in the buffer, the
 *      buffered message may be be overwritten if a new message has arrived.
 */
    void process_cdc_input();

/**
 * \brief move the current CPU time to the timestamp registers.
 * \warning must be called before timestamp registers are read.
 */
    void update_timestamp_regs();

/**
 * \brief return a reference to the specified core or app register's specs used
 *  for issuing a harp reply for that register.
 * \details address	is the full address range where 0 is the first core
 *  register, and APP_REG_START_ADDRESS is the first app register.
 */
    const RegSpecs& reg_address_to_specs(uint8_t address);

    // core register read handler functions. Handles read operations on those
    // registers. One-per-harp-register where necessary, but read_reg_generic()
    // can be used in most cases.
    // Note: these all need to have the same function signature.
    static void read_timestamp_second(uint8_t reg_name);
    static void read_timestamp_microsecond(uint8_t reg_name);


    // write handler function per core register. Handles write
    // operations to that register.
    // Note: these all need to have the same function signature.
    static void write_timestamp_second(msg_t& msg);
    static void write_timestamp_microsecond(msg_t& msg);
    static void write_operation_ctrl(msg_t& msg);
    static void write_reset_def(msg_t& msg);
    static void write_device_name(msg_t& msg);
    static void write_serial_number(msg_t& msg);
    static void write_clock_config(msg_t& msg);
    static void write_timestamp_offset(msg_t& msg);

    Registers regs_; ///< struct of Harp core registers

/**
 * \brief Function table containing the read/write handler functions, one pair
 *  per core register. Index is the register address.
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
