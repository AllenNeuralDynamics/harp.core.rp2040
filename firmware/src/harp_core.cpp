#include <harp_core.h>

HarpCore& HarpCore::init(uint16_t who_am_i, uint16_t hw_version,
                         uint8_t assembly_version, uint16_t harp_version,
                         uint16_t fw_version)
{
    // Create the singleton instance using the private constructor.
    static HarpCore core_(who_am_i, hw_version, assembly_version, harp_version,
                         fw_version);
    return core_;
}

HarpCore::HarpCore(uint16_t who_am_i, uint16_t hw_version,
                     uint8_t assembly_version, uint16_t harp_version,
                     uint16_t fw_version)
:regs_{who_am_i, hw_version, assembly_version, harp_version, fw_version}
{
// TODO: Consider making the rest of this boilerplate setup virtual so it can
//  be device-agnostic.


// Configure USB-Serial
// TODO: figure out if TinyUSB has a sendnow feature.
// Setup DMA for moving data from registers to serial port TX.
}

HarpCore::~HarpCore(){}

void HarpCore::handle_rx_buffer_message()
{
    // Reinterpret contents of the uart rx buffer as a message and dispatch it.
    // Use references and ptrs so that we don't actually copy anything.
    msg_header_t& header = *((msg_header_t*)(&rx_buffer_));
    void* payload = rx_buffer_ + header.payload_base_index_offset();
    uint8_t& checksum = *(rx_buffer_ + header.checksum_index_offset());
    // TODO: handle error checking.

    // Note: controller-to-device protocol interactions are such that we should
    //  never have this situation. Nevertheless, let's handle it and just
    //  ignore the timestamp information.
    if (header.has_timestamp())
    {
        uint32_t timestamp_sec = uint32_t(*(rx_buffer_ + 5));
        uint16_t timestamp_usec = uint16_t(*(rx_buffer_ + 9));
        timestamped_msg_t msg{header, timestamp_sec, timestamp_usec,
                              payload, checksum};

        printf("I am a Pi pico with ID: %d. Data from this message (%d bytes) is: \r\n",
               regs_.regs_.R_WHO_AM_I, msg.payload_length());
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

        printf("I am a Pi pico with ID: %d. Data from this message (%d bytes) is: \r\n",
               regs_.regs_.R_WHO_AM_I, msg.payload_length());
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
    // Construct Harp Reply that includes timestamp.
    uint8_t raw_length = specs.num_bytes + 10;
    uint8_t checksum = 0;
    msg_header_t header{READ, raw_length, (RegNames)reg_name, 255,
                        specs.payload_type};
    // Push data into usb packet and send it.
    for (uint8_t i = 0; i < sizeof(header); ++i)  // push the header.
    {
        uint8_t& byte = *(((uint8_t*)(&header))+i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    for (uint8_t i = 0; i < specs.num_bytes; ++i) // push the payload data.
    {
        const volatile uint8_t& byte = *(specs.base_ptr + i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    tud_cdc_write_char(checksum);
    tud_cdc_write_flush();  // Send usb packet, even if not full.
}

void HarpCore::write_to_read_only_reg_error(msg_t& msg)
{
#ifdef DEBUG
    printf("Error: Reg address %d is read-only.\r\n", &msg.address);
#endif
}

void HarpCore::read_timestamp_second(RegNames reg_name)
{
    regs_.regs_.R_TIMESTAMP_SECOND = time_us_64() / 1000000ULL;
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

    // issue a harp reply via a write_reg_generic?
}

void HarpCore::read_timestamp_microsecond(RegNames reg_name)
{
    // Update register. Then trigger a generic register read.
    regs_.regs_.R_TIMESTAMP_MICRO = uint16_t(timer_hw->timelr >> 5);
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

