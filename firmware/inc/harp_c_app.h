#ifndef HARP_C_APP_H
#define HARP_C_APP_H
#include <harp_core.h>
#include <core_registers.h>
#include <reg_types.h>
//#include <harp_message.h>

/*
 * \brief Data structure for aggregating read/write function pointers.
 */
struct RegFnPair
{
    void (*read_fn_ptr)(uint8_t);  // reg name (enum)
    void (*write_fn_ptr)(msg_t&);
};

/**
 * \brief Harp C-style App that handles core behaviors in addition t
*       reads/writes to app-specific registers.
*       Implemented as a singleton to simplify attaching interrupt callbacks
*       (and since you can only have one per device).
 */
class HarpCApp: public HarpCore
{

// Make constructor private to prevent creating instances outside of init().
private:
/**
 * \brief constructor
 * \param app_reg_values pointer to struct containing registers.
 * \param app_reg_specs array of reg specs, indexed by app register address.
 * \param app_register_count number of app registers
 * \param reg_fns array of RegFnPairs {read fn ptr, write fn ptr}, indexed by
 *  register address.
 */
    HarpCApp(uint16_t who_am_i,
             uint8_t hw_version_major, uint8_t hw_version_minor,
             uint8_t assembly_version,
             uint8_t harp_version_major, uint8_t harp_version_minor,
             uint8_t fw_version_major, uint8_t fw_version_minor,
             uint16_t serial_number, const char name[],
             void* app_reg_values, RegSpecs* app_reg_specs,
             RegFnPair* reg_fns, size_t app_reg_count,
             void (* update_fn)(void));

    ~HarpCApp();

public:
    HarpCApp() = delete;  // Disable default constructor.
    HarpCApp(HarpCApp& other) = delete; // Disable copy constructor.
    void operator=(const HarpCApp& other) = delete; // Disable assignment operator.

/**
 * \brief initialize the harp core app singleton with parameters and init Tinyusb.
 */
    static HarpCApp& init(uint16_t who_am_i,
                          uint8_t hw_version_major, uint8_t hw_version_minor,
                          uint8_t assembly_version,
                          uint8_t harp_version_major, uint8_t harp_version_minor,
                          uint8_t fw_version_major, uint8_t fw_version_minor,
                          uint16_t serial_number, const char name[],
                          void* app_reg_values, RegSpecs* app_reg_specs,
                          RegFnPair* reg_fns, size_t app_reg_count,
                          void (* update_fn)(void));

    static inline HarpCApp* self = nullptr;
    static HarpCApp& instance() {return *self;}

/**
 * \brief Periodically handle tasks based on the current time, state,
 *      and inputs. Should be called in a loop.
 */
    void run();

/**
 * \brief entry point for handling incoming harp messages to core registers.
 *      Dispatches message to the appropriate handler.
 */
    void handle_buffered_app_message();

/**
 *
 */
    void update_state();

/**
 *
 */
    static void read_app_reg_generic(uint8_t reg_name);

/**
 *
 */
    static void write_app_reg_generic(msg_t& msg);

private:

// Private Members
    void* reg_values_;
    RegSpecs* reg_specs_;
    RegFnPair* reg_fns_;
    size_t reg_count_;
    void (* update_fn_)(void);
};

#endif //HARP_C_APP_H
