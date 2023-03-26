#include <harp_core.h>

HarpCore& HarpCore::init(uint16_t who_am_i,
                         uint8_t hw_version_major, uint8_t hw_version_minor,
                         uint8_t assembly_version,
                         uint8_t harp_version_major, uint8_t harp_version_minor,
                         uint8_t fw_version_major, uint8_t fw_version_minor,
                         const char name[])
{
    // Create the singleton instance using the private constructor.
    static HarpCore core(who_am_i, hw_version_major, hw_version_minor,
                          assembly_version,
                          harp_version_major, harp_version_minor,
                          fw_version_major, fw_version_minor, name);
    return core;
}

HarpCore::HarpCore(uint16_t who_am_i,
                   uint8_t hw_version_major, uint8_t hw_version_minor,
                   uint8_t assembly_version,
                   uint8_t harp_version_major, uint8_t harp_version_minor,
                   uint8_t fw_version_major, uint8_t fw_version_minor,
                   const char name[])
:regs_{who_am_i, hw_version_major, hw_version_minor,assembly_version,
       harp_version_major, harp_version_minor,
       fw_version_major, fw_version_minor, name},
 rx_buffer_index_{0}, total_bytes_read_{rx_buffer_index_}, new_msg_{false}
{
    // Create a pointer to the first (and one-and-only) instance created.
    if (self == nullptr)
        self = this;
}

HarpCore::~HarpCore(){self = nullptr;}

void HarpCore::run()
{
    update_state();
    process_cdc_input();
    if (not new_msg_)
        return;
    // Handle in-range register msgs. Ignore msgs outside our range.
    msg_header_t& header = get_buffered_msg_header();
    if (header.address < CORE_REG_COUNT)
        handle_buffered_core_message();
}

void HarpCore::process_cdc_input()
{
    // TODO: Consider a timeout if we never receive a fully formed message.
    // Fetch all data in the serial port. If it's at least a header's worth,
    // check the payload size and keep reading up to the end of the packet.
    if (not tud_cdc_available())
        return;
    // If the header has arrived, only read up to the full payload so we can
    // process one message at a time.
    uint32_t max_bytes_to_read = sizeof(rx_buffer_) - rx_buffer_index_;
    if (total_bytes_read_ >= sizeof(msg_header_t))
    {
        // Reinterpret contents of the rx buffer as a message header.
        msg_header_t& header = get_buffered_msg_header();
        // Read only the remainder of a single harp message.
        max_bytes_to_read = header.msg_size() - total_bytes_read_;
    }
    uint32_t bytes_read = tud_cdc_read(&(rx_buffer_[rx_buffer_index_]),
                                       max_bytes_to_read);
    rx_buffer_index_ += bytes_read;
    // See if we have a message header's worth of data yet. Baily early if not.
    if (total_bytes_read_ < sizeof(msg_header_t))
        return;
    // Reinterpret contents of the rx buffer as a message header.
    msg_header_t& header = get_buffered_msg_header();
    // Bail early if the full message (with payload) has not fully arrived.
    if (total_bytes_read_ < header.msg_size())
        return;
    rx_buffer_index_ = 0; // Reset buffer index for the next message.
    new_msg_ = true;
    return;
}

msg_t HarpCore::get_buffered_msg()
{
    // Reinterpret contents of the uart rx buffer as a message and dispatch it.
    // Use references and ptrs so that we don't make any copies.
    msg_header_t& header = get_buffered_msg_header();
    void* payload = rx_buffer_ + header.payload_base_index_offset();
    uint8_t& checksum = *(rx_buffer_ + header.checksum_index_offset());
    return msg_t{header, payload, checksum};
}

void HarpCore::handle_buffered_core_message()
{
    msg_t msg = get_buffered_msg();
    // TODO: check checksum.
    // Note: PC-to-Harp msgs don't have timestamps, so we don't check for them.
/*
        printf("Data from this message (%d bytes) is: \r\n",
               msg.payload_length());
        printf("msg_type: %d\r\n", msg.header.type);
        printf("raw_length: %d\r\n", msg.header.raw_length);
        printf("address: %d\r\n", msg.header.address);
        printf("port: %d\r\n", msg.header.port);
        printf("payload_type: %d\r\n", msg.header.payload_type);
        for (auto i = 0; i < msg.payload_length(); ++i)
            printf("%d, ", ((uint8_t*)(msg.payload))[i]);
        printf("\r\n");
*/
    // Handle read-or-write behavior.
    switch (msg.header.type)
    {
        case READ:
            return read_from_reg((RegName)msg.header.address);
        case WRITE:
            return write_to_reg(msg);
        default:
            break;
    }
    clear_msg();
}

void HarpCore::update_state()
{
    uint32_t curr_time_us = time_us_32();
    //switch (regs.R_OPERATION_CTRL & 0x03) // op mode
    switch (regs_.r_operation_ctrl_bits.OP_MODE)
    {
        case STANDBY:
            break;
        case ACTIVE:
            // Drop to STANDBY mode if we've been in active mode for too long
            // without communication.
            break;
        case RESERVED:
            break;
        case SPEED:
            break;
        default:
            return;
    }
    // Handle state LED.
}

void HarpCore::send_harp_reply(msg_type_t reply_type, RegName reg_name,
                               const volatile uint8_t* data, uint8_t num_bytes,
                               reg_type_t payload_type)
{
    // FIXME: implementation as-is cannot send more than 64 bytes of data
    //  because of underlying usb implementation.
    // Dispatch timestamped Harp reply.
    // Note: This fn implementation assumes little-endian architecture.
    uint8_t raw_length = num_bytes + 10;
    uint8_t checksum = 0;
    msg_header_t header{reply_type, raw_length, reg_name, 255,
                        (reg_type_t)(HAS_TIMESTAMP | payload_type)};
    // Push data into usb packet and send it.
    for (uint8_t i = 0; i < sizeof(header); ++i) // push the header.
    {
        uint8_t& byte = *(((uint8_t*)(&header))+i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    update_timestamp_regs(); // update and push timestamp in required order.
    for (uint8_t i = 0; i < sizeof(regs.R_TIMESTAMP_SECOND); ++i)
    {
        uint8_t& byte = *(((uint8_t*)(&regs.R_TIMESTAMP_SECOND)) + i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    for (uint8_t i = 0; i < sizeof(regs.R_TIMESTAMP_MICRO); ++i)
    {
        uint8_t& byte = *(((uint8_t*)(&regs.R_TIMESTAMP_MICRO)) + i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    // TODO: should we lockout global interrupts to prevent reg data from
    //  changing underneath us?
    for (uint8_t i = 0; i < num_bytes; ++i) // push the payload data.
    {
        const volatile uint8_t& byte = *(data + i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    tud_cdc_write_char(checksum); // push the checksum.
    tud_cdc_write_flush();  // Send usb packet, even if not full.
}

void HarpCore::read_reg_generic(RegName reg_name)
{
    const RegSpecs& specs = regs_.enum_to_reg_specs[reg_name];
    send_harp_reply(READ, reg_name, specs.base_ptr, specs.num_bytes,
                    specs.payload_type);
}

void HarpCore::write_to_read_only_reg_error(msg_t& msg)
{
#ifdef DEBUG
    printf("Error: Reg address %d is read-only.\r\n", &msg.address);
#endif
//    send_harp_reply(ERROR);
}

void HarpCore::update_timestamp_regs()
{
    // PICO implementation:
    //  extract time data from pico timer which increments every 1[us].
    // Note that R_TIMESTAMP_MICRO can only represent values up to 31249.
    // Update microseconds first.
    //regs.R_TIMESTAMP_MICRO = uint16_t(time_us_32() >> 5)%31250;
    regs.R_TIMESTAMP_MICRO = uint16_t((time_us_32()%1000000U)>>5);
    regs.R_TIMESTAMP_SECOND = time_us_64() / 1000000ULL;
}

void HarpCore::read_timestamp_second(RegName reg_name)
{
    update_timestamp_regs();
    read_reg_generic(reg_name);
}

void HarpCore::write_timestamp_second(msg_t& msg)
{
    const uint32_t& seconds = *((uint32_t*)msg.payload);
    // PICO implementation: replace the current number of elapsed seconds
    // without altering the number of elapsed microseconds.
    uint64_t set_time_microseconds = uint64_t(seconds) * 1000000UL;
    uint32_t current_microseconds = time_us_64() % 1000000ULL;
    uint64_t new_time = set_time_microseconds + current_microseconds;
    // Set the low and high time registers with the desired seconds.
    // Update low register first.
    timer_hw->timelw = (uint32_t)new_time;  // Truncate.
    timer_hw->timehw = (uint32_t)(new_time >> 32);

    // TODO: issue a harp reply via some sort of write_reg_generic?
}

void HarpCore::read_timestamp_microsecond(RegName reg_name)
{
    // Update register. Then trigger a generic register read.
    update_timestamp_regs();
    read_reg_generic(reg_name);
}

void HarpCore::write_timestamp_microsecond(msg_t& msg)
{
    const uint32_t& microseconds = ((uint32_t)(*((uint16_t*)msg.payload))) << 5;
    // PICO implementation: replace the current number of elapsed microseconds.
    timer_hw->timelw = (time_us_32() & ~0x001FFFFF) +  microseconds;
}

void HarpCore::write_operation_ctrl(msg_t& msg)
{
    uint8_t& write_byte = *((uint8_t*)msg.payload);
    // Update register state. Note: DUMP bit always reads as zero.
    regs.R_OPERATION_CTRL = write_byte & ~(0x01 << DUMP_OFFSET);
    // Tease out flags.
    bool DUMP = bool((write_byte >> DUMP_OFFSET) & 0x01);
    // Bail early if we are muted.
    if (is_muted())
        return;
    // Send reply. If DUMP: reply is all registers serialized (little-endian).
    const RegSpecs& specs = regs_.enum_to_reg_specs[msg.header.address];
    const RegName& reg_name = (RegName)msg.header.address;
    if (DUMP)
        send_harp_reply(WRITE, reg_name, (uint8_t*)&regs, sizeof(regs), U8);
    else
        send_harp_reply(WRITE, reg_name, specs.base_ptr, specs.num_bytes,
                        specs.payload_type);
}

void HarpCore::write_reset_def(msg_t& msg)
{
    // TODO.
    if (is_muted())
        return;
}

void HarpCore::write_device_name(msg_t& msg)
{
    // TODO.
    // PICO implementation. Write to allocated flash memory
    // since we have no eeprom.
// https://github.com/raspberrypi/pico-examples/blob/master/flash/program/flash_program.c
    if (is_muted())
        return;
}

void HarpCore::write_serial_number(msg_t& msg)
{
    // TODO.
    if (is_muted())
        return;
}

void HarpCore::write_clock_config(msg_t& msg)
{
    // TODO.
    if (is_muted())
        return;
}

void HarpCore::write_timestamp_offset(msg_t& msg)
{
    // TODO.
    if (is_muted())
        return;
}

