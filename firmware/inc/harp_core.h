#ifndef HARP_CORE_H
#define HARP_CORE_H
#include <stdint.h>
#include <harp_message.h>
#include <registers.h>
#include <functional>  // for std::invoke


struct HarpCore
{
    public:
        HarpCore(uint16_t who_am_i, uint16_t hw_version,
                  uint8_t assembly_version, uint16_t harp_version,
                  uint16_t fw_version);
        ~HarpCore();

    // Create a typedef such that we can create an array of member fn ptrs.
    // MsgHandleMemberFn points to a harp core member fn that takes (msg_t&)
    // https://isocpp.org/wiki/faq/pointers-to-members#array-memfnptrs
    typedef void (HarpCore::*MsgHandleMemberFn)(msg_t& mst);

/**
 * \brief entry point for handling incoming harp messages. Dispatches message
 *      to the appropriate handler.
 */
    void handle_message(msg_t& msg);

/**
 * \brief a handler function per harp register. Handles read and write
 *      operations to that register.
 */
    void handle_who_am_i(msg_t& msg);
    void handle_hw_version_h(msg_t& msg);
    void handle_hw_version_l(msg_t& msg);
    void handle_assembly_version(msg_t& msg);
    void handle_harp_version_h(msg_t& msg);
    void handle_harp_version_l(msg_t& msg);
    void handle_fw_version_h(msg_t& msg);
    void handle_fw_version_l(msg_t& msg);
    void handle_timestamp_second(msg_t& msg);
    void handle_timestamp_microsecond(msg_t& msg);
    void handle_operation_ctrl(msg_t& msg);
    void handle_reset_def(msg_t& msg);
    void handle_device_name(msg_t& msg);
    void handle_serial_number(msg_t& msg);
    void handle_clock_config(msg_t& msg);
    void handle_timestamp_offset(msg_t& msg);

    Registers regs_;

    // Function Table. Order matters since we will index into it with enums.
    MsgHandleMemberFn reg_handler_fns_[REG_COUNT] =
    {&HarpCore::handle_who_am_i,
     &HarpCore::handle_hw_version_h,
     &HarpCore::handle_hw_version_l,
     &HarpCore::handle_assembly_version,
     &HarpCore::handle_harp_version_h,
     &HarpCore::handle_harp_version_l,
     &HarpCore::handle_fw_version_h,
     &HarpCore::handle_fw_version_l,
     &HarpCore::handle_timestamp_second,
     &HarpCore::handle_timestamp_microsecond,
     &HarpCore::handle_operation_ctrl,
     &HarpCore::handle_reset_def,
     &HarpCore::handle_device_name,
     &HarpCore::handle_serial_number,
     &HarpCore::handle_clock_config,
     &HarpCore::handle_timestamp_offset,
    };
};

#endif //HARP_CORE_H
