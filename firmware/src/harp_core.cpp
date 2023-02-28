#include <harp_core.h>

HarpCore::HarpCore(uint16_t who_am_i, uint16_t hw_version,
                     uint8_t assembly_version, uint16_t harp_version,
                     uint16_t fw_version)
:regs_{who_am_i, hw_version, assembly_version, harp_version, fw_version}
{

// Setup FTDI-based hardware uart with hardware flow control enabled.
// Trigger an interrupt wh

// Setup SYSTICK to count at 1[us] intervals up to 1e6.
// https://electronics.stackexchange.com/questions/305423/how-do-i-set-systick-to-1-ms
    // TODO
// Setup SYSTICK to trigger an interrupt a 1[us] intervals to update the time.
    SYST_CSRbits.ENABLE = 1;
}

HarpCore::~HarpCore(){}

void HarpCore::handle_rx_buffer_message()
{
    // Reinterpret contents of the uart rx buffer as a message and dispatch it.
    // Use references and ptrs so that we don't actually copy anything.
    msg_header_t& header = *((msg_header_t*)(&rx_buffer_));
    void* payload = rx_buffer_ + header.payload_base_index_offset();
    uint8_t& checksum = *(rx_buffer_ + header.checksum_index_offset());
    /*
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
    // TODO: reply to this message by queuing RX data out.
    */
    // TODO: handle error checking.

    if (header.has_timestamp())
    {
        uint32_t timestamp_sec = uint32_t(*(rx_buffer_ + 5));
        uint16_t timestamp_usec = uint16_t(*(rx_buffer_ + 9));
        timestamped_msg_t msg{header, timestamp_sec, timestamp_usec,
                              payload, checksum};
        printf("address of header: %d | ", &header);
        printf("address of msg.header: %d\r\n", &(msg.header));
        printf("address of checksum: %d | ", &checksum);
        printf("address of checksum: %d | ", &(msg.checksum));
        // Handle read-or-write behavior.
        switch (msg.header.type)
        {
            case READ:
                return read_from_reg(msg.header.address);
            case WRITE:
                return write_to_reg(msg);
            default:
                break;
        }
    }
    else
    {
        msg_t msg{header, payload, checksum};
        printf("address of header: %d | ", &header);
        printf("address of msg.header: %d\r\n", &(msg.header));
        printf("address of checksum: %d | ", &checksum);
        printf("address of checksum: %d | ", &(msg.checksum));
        // Handle read-or-write behavior.
        switch (msg.header.type)
        {
            case READ:
                return read_from_reg(msg.header.address);
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
    //push_reg_data_to_tx_buffer(reg_name, specs.base_ptr, specs.num_bytes);
    // TODO: push data to tx buffer via DMA.
    // Ensure DMA is not busy.
}

void HarpCore::write_to_read_only_reg_error(msg_t& msg)
{
#ifdef DEBUG
    printf("Error: Reg address %d is read-only.\r\n", &msg.address);
#endif
}

void HarpCore::write_timestamp_second(msg_t& msg)
{
    SYST_CSRbits.TICKINT = 0; // Disable SYSTICK-generated interrupt.
    regs_.regs_.R_TIMESTAMP_SECOND = *((uint32_t*)msg.payload);
    SYST_CSRbits.TICKINT = 1; // Enable SYSTICK-generated interrupt.
}

void HarpCore::read_timestamp_microsecond(RegNames reg_name)
{
    // Pull microsecond data from SYSTICK; push into register.
    // TODO: check math here..
    regs_.regs_.R_TIMESTAMP_MICRO = uint16_t((999999 - SYST_CVR) >> 5);
    read_reg_generic(reg_name);
}

void HarpCore::write_timestamp_microsecond(msg_t& msg)
{
    // Update SYSTICK current value. (Remember that it counts DOWN.)
    // TODO: check math.
    SYST_CVRbits.CURRENT = 1e6 - ((uint32_t)(*((uint16_t*)msg.payload)) << 5);
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

