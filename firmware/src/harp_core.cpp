#include <harp_core.h>

HarpCore::HarpCore(uint16_t who_am_i, uint16_t hw_version,
                     uint8_t assembly_version, uint16_t harp_version,
                     uint16_t fw_version)
:regs_{who_am_i, hw_version, assembly_version, harp_version, fw_version}
{}

HarpCore::~HarpCore(){}

void HarpCore::handle_who_am_i(msg_t& msg)
{
    printf("I am a Pi pico.\r\n");
}

void HarpCore::handle_hw_version_h(msg_t& msg_t)
{
}

void HarpCore::handle_hw_version_l(msg_t& msg_t)
{
}

void HarpCore::handle_assembly_version(msg_t& msg_t)
{
}

void HarpCore::handle_harp_version_h(msg_t& msg_t)
{
}

void HarpCore::handle_harp_version_l(msg_t& msg_t)
{
}

void HarpCore::handle_fw_version_h(msg_t& msg_t)
{
}

void HarpCore::handle_fw_version_l(msg_t& msg_t)
{
}

void HarpCore::handle_timestamp_second(msg_t& msg_t)
{
}

void HarpCore::handle_timestamp_microsecond(msg_t& msg)
{
}

void HarpCore::handle_operation_ctrl(msg_t& msg_t)
{
}

void HarpCore::handle_reset_def(msg_t& msg_t)
{
}

void HarpCore::handle_device_name(msg_t& msg_t)
{
}

void HarpCore::handle_serial_number(msg_t& msg_t)
{
}

void HarpCore::handle_clock_config(msg_t& msg_t)
{
}

void HarpCore::handle_timestamp_offset(msg_t& msg_t)
{
}

