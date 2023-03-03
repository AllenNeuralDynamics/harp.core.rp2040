#include <harp_core.h>

HarpCore& HarpCore::init(uint16_t who_am_i,
                         uint8_t hw_version_major, uint8_t hw_version_minor,
                         uint8_t assembly_version,
                         uint8_t harp_version_major, uint8_t harp_version_minor,
                         uint8_t fw_version_major, uint8_t fw_version_minor,
                         const char name[])
{
    // Create the singleton instance using the private constructor.
    static HarpCore core_(who_am_i, hw_version_major, hw_version_minor,
                          assembly_version,
                          harp_version_major, harp_version_minor,
                          fw_version_major, fw_version_minor, name);
    return core_;
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
 rx_buffer_index_{0}
{
// TODO: Consider making the rest of this boilerplate setup virtual so it can
//  be fully device-agnostic.

// Configure USB-Serial
}

HarpCore::~HarpCore(){}


void HarpCore::handle_rx_buffer_input()
{
    // Fetch all data in the serial port. If it's at least a header's worth,
    // start processing the message. Dispatch msg if it has completely arrived.
    uint new_byte = getchar_timeout_us(0);
    while (new_byte != PICO_ERROR_TIMEOUT)
    {
        rx_buffer_[rx_buffer_index_++] = uint8_t(new_byte);
        new_byte = getchar_timeout_us(0);
    }
    // See if we have payload length's worth of data yet. Baily early if not.
    uint8_t bytes_read = rx_buffer_index_ + 1;
    if (bytes_read < sizeof(msg_header_t))
        return;
    msg_header_t& header = *((msg_header_t*)(&rx_buffer_));
    if (bytes_read < header.raw_length + 2)
        return;
    rx_buffer_index_ = 0; // Reset buffer index for next message.
    // Process the fully-formed message.
    handle_rx_buffer_message();
}

void HarpCore::handle_rx_buffer_message()
{
    // Reinterpret contents of the uart rx buffer as a message and dispatch it.
    // Use references and ptrs so that we don't make any copies.
    msg_header_t& header = *((msg_header_t*)(&rx_buffer_));
    void* payload = rx_buffer_ + header.payload_base_index_offset();
    uint8_t& checksum = *(rx_buffer_ + header.checksum_index_offset());
    // TODO: check checksum.

    // Note: controller-to-device protocol interactions are such that we should
    //  never have this situation. Nevertheless, let's handle it and just
    //  ignore the timestamp information.
    if (header.has_timestamp())
    {
        uint32_t timestamp_sec = uint32_t(*(rx_buffer_ + 5));
        uint16_t timestamp_usec = uint16_t(*(rx_buffer_ + 9));
        timestamped_msg_t msg{header, timestamp_sec, timestamp_usec,
                              payload, checksum};

/*
        printf("I am a Pi pico with ID: %d. Data from this message (%d bytes) is: \r\n",
               regs.R_WHO_AM_I, msg.payload_length());
        printf("msg_type: %d\r\n", msg.header.type);
        printf("raw_length: %d\r\n", msg.header.raw_length);
        printf("address: %d\r\n", msg.header.address);
        printf("port: %d\r\n", msg.header.port);
        printf("payload_type: %d\r\n", msg.header.payload_type);
        for (auto i = 0; i < msg.payload_length(); ++i)
            printf("%d, ", ((uint8_t*)(msg.payload))[i]);
        printf("\r\n");

        printf("address of header: %d | ", &header);
        printf("address of msg.header: %d\r\n", &(msg.header));
        printf("address of checksum: %d | ", &checksum);
        printf("address of checksum: %d | ", &(msg.checksum));
*/
        // Handle read-or-write behavior.
        switch (msg.header.type)
        {
            case READ:
                return read_from_reg((RegNames)msg.header.address);
            case WRITE:
                return write_to_reg(msg);
            default:
                break;
        }
    }
    else
    {
        msg_t msg{header, payload, checksum};
/*
        printf("I am a Pi pico with ID: %d. Data from this message (%d bytes) is: \r\n",
               regs.R_WHO_AM_I, msg.payload_length());
        printf("msg_type: %d\r\n", msg.header.type);
        printf("raw_length: %d\r\n", msg.header.raw_length);
        printf("address: %d\r\n", msg.header.address);
        printf("port: %d\r\n", msg.header.port);
        printf("payload_type: %d\r\n", msg.header.payload_type);
        for (auto i = 0; i < msg.payload_length(); ++i)
            printf("%d, ", ((uint8_t*)(msg.payload))[i]);
        printf("\r\n");

        printf("address of header: %d | ", &header);
        printf("address of msg.header: %d\r\n", &(msg.header));
        printf("address of checksum: %d | ", &checksum);
        printf("address of checksum: %d | ", &(msg.checksum));
*/
        // Handle read-or-write behavior.
        switch (msg.header.type)
        {
            case READ:
                return read_from_reg((RegNames)msg.header.address);
            case WRITE:
                return write_to_reg(msg);
            default:
                break;
        }
    }
}

void HarpCore::read_reg_generic(RegNames reg_name)
{
    const RegSpecs& specs = regs_.enum_to_reg_specs[reg_name];
    // Dispatch serialized Harp reply that includes timestamp.
    uint8_t raw_length = specs.num_bytes + 10;
    uint8_t checksum = 0;
    msg_header_t header{READ, raw_length, (RegNames)reg_name, 255,
                        (payload_type_t)(HAS_TIMESTAMP | specs.payload_type)};
    // FIXME: arm devices are little-endian.
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
    for (uint8_t i = 0; i < specs.num_bytes; ++i) // push the payload data.
    {
        const volatile uint8_t& byte = *(specs.base_ptr + i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    tud_cdc_write_char(checksum); // push the checksum.
    tud_cdc_write_flush();  // Send usb packet, even if not full.
}

void HarpCore::write_to_read_only_reg_error(msg_t& msg)
{
#ifdef DEBUG
    printf("Error: Reg address %d is read-only.\r\n", &msg.address);
#endif
}

void HarpCore::update_timestamp_regs()
{
    // PICO implementation:
    // Update microseconds first.
    regs.R_TIMESTAMP_MICRO = uint16_t(timer_hw->timelr >> 5);
    regs.R_TIMESTAMP_SECOND = time_us_64() / 1000000ULL;
}

void HarpCore::read_timestamp_second(RegNames reg_name)
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

    // TODO: do we need to issue a harp reply via some sort of write_reg_generic?
}

void HarpCore::read_timestamp_microsecond(RegNames reg_name)
{
    // Update register. Then trigger a generic register read.
    update_timestamp_regs();
    read_reg_generic(reg_name);
}

void HarpCore::write_timestamp_microsecond(msg_t& msg)
{
    const uint32_t& microseconds = ((uint32_t)(*((uint16_t*)msg.payload))) << 5;
    // PICO implementation: replace the current number of elapsed microseconds.
    timer_hw->timelw = (timer_hw->timelr & 0x001FFFFF) +  microseconds;
}

void HarpCore::write_operation_ctrl(msg_t& msg_t)
{
}

void HarpCore::write_reset_def(msg_t& msg_t)
{
}

void HarpCore::write_device_name(msg_t& msg_t)
{
    // PICO implementation. Write to allocated flash memory
    // since we have no eeprom.
// https://github.com/raspberrypi/pico-examples/blob/master/flash/program/flash_program.c
}

void HarpCore::write_serial_number(msg_t& msg_t)
{
}

void HarpCore::write_clock_config(msg_t& msg_t)
{
}

void HarpCore::write_timestamp_offset(msg_t& msg_t)
{
}

