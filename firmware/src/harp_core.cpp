#include <harp_core.h>

HarpCore::HarpCore(uint16_t who_am_i, uint16_t hw_version,
                     uint8_t assembly_version, uint16_t harp_version,
                     uint16_t fw_version)
:regs_{who_am_i, hw_version, assembly_version, harp_version, fw_version}
{}

HarpCore::~HarpCore(){}

void HarpCore::handle_message(msg_t& msg)
{
    std::invoke(reg_handler_fns_[msg.address], this, msg);
}

void HarpCore::handle_who_am_i(msg_t& msg)
{
}

void HarpCore::handle_timestamp_microsecond(msg_t& msg)
{
}
