#include <harp_core.h>

HarpCore& HarpCore::init(uint16_t who_am_i,
                         uint8_t hw_version_major, uint8_t hw_version_minor,
                         uint8_t assembly_version,
                         uint8_t harp_version_major, uint8_t harp_version_minor,
                         uint8_t fw_version_major, uint8_t fw_version_minor,
                         uint16_t serial_number, const char name[])
{
    // Create the singleton instance using the private constructor.
    static HarpCore core(who_am_i, hw_version_major, hw_version_minor,
                         assembly_version,
                         harp_version_major, harp_version_minor,
                         fw_version_major, fw_version_minor, serial_number,
                         name);
    return core;
}

HarpCore::HarpCore(uint16_t who_am_i,
                   uint8_t hw_version_major, uint8_t hw_version_minor,
                   uint8_t assembly_version,
                   uint8_t harp_version_major, uint8_t harp_version_minor,
                   uint8_t fw_version_major, uint8_t fw_version_minor,
                   uint16_t serial_number, const char name[])
:regs_{who_am_i, hw_version_major, hw_version_minor,assembly_version,
       harp_version_major, harp_version_minor,
       fw_version_major, fw_version_minor, serial_number, name},
 rx_buffer_index_{0}, total_bytes_read_{rx_buffer_index_}, new_msg_{false}
{
    // Create a pointer to the first (and one-and-only) instance created.
    if (self == nullptr)
        self = this;
    tusb_init();
}

HarpCore::~HarpCore(){self = nullptr;}

void HarpCore::run()
{
    tud_task();
    update_state();
    update_app_state(); // Does nothing unless a derived class implements it.
    process_cdc_input();
    if (not new_msg_)
        return;
#ifdef DEBUG_HARP_MSG_IN
    msg_t msg = get_buffered_msg();
    printf("Msg data: \r\n");
    printf("  type: %d\r\n", msg.header.type);
    printf("  addr: %d\r\n", msg.header.address);
    printf("  raw len: %d\r\n", msg.header.raw_length);
    printf("  port: %d\r\n", msg.header.port);
    printf("  payload type: %d\r\n", msg.header.payload_type);
    printf("  payload len: %d\r\n", msg.header.payload_length());
    uint8_t payload_len = msg.header.payload_length();
    if (payload_len > 0)
    {
        printf("  payload: ");
        for (auto i = 0; i < msg.payload_length(); ++i)
            printf("%d, ", ((uint8_t*)(msg.payload))[i]);
    }
    printf("\r\n\r\n");
#endif
    // Handle in-range register msgs and clear them. Ignore out-of-range msgs.
    handle_buffered_core_message(); // Handle msg. Clear it if handled.
    if (not new_msg_)
        return;
    handle_buffered_app_message(); // Handle msg. Clear it if handled.
    // Always clear any unhandled messages, so we don't lock up.
    if (new_msg_)
    {
#ifdef DEBUG_HARP_MSG_IN
    printf("Ignoring out-of-range msg!\r\n");
#endif
        clear_msg();
    }
}

void HarpCore::process_cdc_input()
{
    // TODO: Consider a timeout if we never receive a fully formed message.
    // TODO: scan for partial messages.
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
    // Reinterpret (i.e: type pun) contents of the uart rx buffer as a message.
    // Use references and ptrs to existing data so we don't make any copies.
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
    // Ignore out-of-range messages. Expect them to be handled by derived class.
    if (msg.header.address > CORE_REG_COUNT)
        return;
    // Handle read-or-write behavior.
    switch (msg.header.type)
    {
        case READ:
            reg_func_table_[msg.header.address].read_fn_ptr(msg.header.address);
            break;
        case WRITE:
            reg_func_table_[msg.header.address].write_fn_ptr(msg);
            break;
    }
    clear_msg();
}

void HarpCore::update_state()
{
    // Update internal logic.
    uint32_t curr_time_us = time_us_32();
    if (tud_cdc_connected())
        disconnect_handled_ = false;
    if (!tud_cdc_connected() && !disconnect_handled_)
    {
        disconnect_handled_ = true;
        disconnect_start_time_us_ = curr_time_us;
    }
    // Update state machine "next-state" logic.
    const uint8_t& state = regs_.r_operation_ctrl_bits.OP_MODE;
    uint8_t next_state{state}; // init next state candidate to current state.
    switch (state)
    {
        case STANDBY: // The PC must put the device into ACTIVE mode.
            break;
        case ACTIVE:
            // Drop to STANDBY if we've lost the PC connection for too long.
            if (disconnect_handled_
                && (curr_time_us - disconnect_start_time_us_) >= NO_PC_INTERVAL_US)
                next_state = STANDBY;
            break;
        case RESERVED:
            break;
        case SPEED:
            break;
        default:
            break;
    }
    // Handle in-state dependent output logic.
    // Handle OPERATION_CTRL ALIVE_EN bit behavior.
    if ((state == ACTIVE) // i.e: if events_enabled()
        && regs_.r_operation_ctrl_bits.ALIVE_EN
        && (int32_t(curr_time_us - next_heartbeat_time_us_) >= 0))
    {
        next_heartbeat_time_us_ += HEARTBEAT_INTERVAL_US;
        self->update_timestamp_regs();
        send_harp_reply(EVENT, TIMESTAMP_SECOND);
    }
    // TODO: Handle state LED.
    // Do the state transition.
    regs_.r_operation_ctrl_bits.OP_MODE = next_state;
}

const RegSpecs& HarpCore::reg_address_to_specs(uint8_t address)
{
    if (address < CORE_REG_COUNT)
        return regs_.address_to_specs[address];
    return address_to_app_reg_specs(address); // virtual. Implemented by app.
}

void HarpCore::send_harp_reply(msg_type_t reply_type, uint8_t reg_name,
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
#ifdef DEBUG_HARP_MSG_OUT
    printf("Sending msg: \r\n");
    printf("  type: %d\r\n", header.type);
    printf("  addr: %d\r\n", header.address);
    printf("  raw len: %d\r\n", header.raw_length);
    printf("  port: %d\r\n", header.port);
    printf("  payload type: %d\r\n", header.payload_type);
    printf("  payload len: %d\r\n", header.payload_length());
    uint8_t payload_len = header.payload_length();
    if (payload_len > 0)
    {
        printf("  payload: ");
        for (auto i = 0; i < header.payload_length(); ++i)
            printf("%d, ", data[i]);
    }
    printf("\r\n\r\n");
#endif
    // Push data into usb packet and send it.
    for (uint8_t i = 0; i < sizeof(header); ++i) // push the header.
    {
        uint8_t& byte = *(((uint8_t*)(&header))+i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    self->update_timestamp_regs(); // update and push timestamp in required order.
    for (uint8_t i = 0; i < sizeof(self->regs.R_TIMESTAMP_SECOND); ++i)
    {
        uint8_t& byte = *(((uint8_t*)(&self->regs.R_TIMESTAMP_SECOND)) + i);
        checksum += byte;
        tud_cdc_write_char(byte);
    }
    for (uint8_t i = 0; i < sizeof(self->regs.R_TIMESTAMP_MICRO); ++i)
    {
        uint8_t& byte = *(((uint8_t*)(&self->regs.R_TIMESTAMP_MICRO)) + i);
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
    // Call tud_task to handle case we issue multiple harp replies in a row.
    // FIXME: a better way might be to check tinyusb's internal buffer's
    // remaining space.
    tud_task();
}

void HarpCore::read_reg_generic(uint8_t reg_name)
{
    send_harp_reply(READ, reg_name);
}


void HarpCore::write_reg_generic(msg_t& msg)
{
    copy_msg_payload_to_register(msg);
    if (self->is_muted())
        return;
    const uint8_t& reg_name = msg.header.address;
    const RegSpecs& specs = self->reg_address_to_specs(msg.header.address);
    send_harp_reply(WRITE, reg_name, specs.base_ptr, specs.num_bytes,
                    specs.payload_type);
}

void HarpCore::write_to_read_only_reg_error(msg_t& msg)
{
#ifdef DEBUG_HARP_MSG_IN
    printf("Error: Reg address %d is read-only.\r\n", msg.header.address);
#endif
    send_harp_reply(WRITE_ERROR, msg.header.address);
}

void HarpCore::update_timestamp_regs()
{
    // PICO implementation:
    //  extract time data from pico timer which increments every 1[us].
    // Note that R_TIMESTAMP_MICRO can only represent values up to 31249.
    // Update microseconds first.
    uint64_t curr_total_us = time_us_64();
    uint32_t curr_total_us_32 = (uint32_t)curr_total_us; // truncate.
    // TODO: use divmod_u64u64_rem to do division with remainder once.
    regs.R_TIMESTAMP_MICRO = uint16_t((curr_total_us_32%1000000UL)>>5);
    regs.R_TIMESTAMP_SECOND = time_us_64() / 1000000ULL;
}

void HarpCore::read_timestamp_second(uint8_t reg_name)
{
    self->update_timestamp_regs();
    read_reg_generic(reg_name);
}

void HarpCore::write_timestamp_second(msg_t& msg)
{
    const uint32_t& seconds = *((uint32_t*)msg.payload);
    // PICO implementation: replace the current number of elapsed seconds
    // without altering the number of elapsed microseconds.
    // TODO: use divmod_u64u64_rem to do division with remainder once.
    uint64_t set_time_microseconds = uint64_t(seconds) * 1000000UL;
    uint32_t current_microseconds = time_us_64() % 1000000ULL;
    uint64_t new_time = set_time_microseconds + current_microseconds;
    // Set the low and high time registers with the desired seconds.
    // Time does not update until timehw is written to.
    timer_hw->timelw = (uint32_t)new_time;  // Truncate.
    timer_hw->timehw = (uint32_t)(new_time >> 32);
    // Send harp reply.
    // Note: harp timestamp registers will be updated before being dispatched.
    send_harp_reply(WRITE, msg.header.address);
}

void HarpCore::read_timestamp_microsecond(uint8_t reg_name)
{
    // Update register. Then trigger a generic register read.
    self->update_timestamp_regs();
    read_reg_generic(reg_name);
}

void HarpCore::write_timestamp_microsecond(msg_t& msg)
{
    const uint32_t msg_us = ((uint32_t)(*((uint16_t*)msg.payload))) << 5;
    // PICO implementation: replace the current number of elapsed microseconds
    // with the value received from the message.
    // timelw and timehw need to be written to such that the update takes place.
    uint64_t curr_total_s = time_us_64() / 1000000ULL; // integer division.
    uint64_t new_time = curr_total_s + msg_us;
    timer_hw->timelw = (uint32_t)new_time;
    timer_hw->timehw = (uint32_t)(new_time >> 32);
    // Send harp reply.
    // Note: Harp timestamp registers will be updated before dispatching reply.
    send_harp_reply(WRITE, msg.header.address);
}

void HarpCore::write_operation_ctrl(msg_t& msg)
{
    uint8_t& write_byte = *((uint8_t*)msg.payload);
    // Handle OP Mode state-edge logic here since we can force a state change
    // directly.
    const uint8_t& state = self->regs_.r_operation_ctrl_bits.OP_MODE;
    const uint8_t& next_state = (*((OperationCtrlBits*)(&write_byte))).OP_MODE;
    // Update register state. Note: DUMP bit always reads as zero.
    self->regs.R_OPERATION_CTRL = write_byte & ~(0x01 << DUMP_OFFSET);
    self->set_visual_indicators(bool((write_byte >> VISUAL_EN_OFFSET) & 0x01));
    // Schedule the heartbeat interval.
    if (next_state == ACTIVE)
    {
        // Round *up* to the nearest HEARTBEAT_INTERVAL_US.
        uint32_t curr_time_us = time_us_32();
        uint32_t remainder;
        uint32_t quotient = divmod_u32u32_rem(curr_time_us,
                                              HEARTBEAT_INTERVAL_US,
                                              &remainder);
        self->next_heartbeat_time_us_ = curr_time_us + HEARTBEAT_INTERVAL_US
                                        - remainder;
    }
    // Bail early if we are muted.
    if (self->is_muted())
        return;
    // Tease out flags.
    bool DUMP = bool((write_byte >> DUMP_OFFSET) & 0x01);
    // Send WRITE reply.
    send_harp_reply(WRITE, msg.header.address);
    // DUMP-bit-specific behavior: if set, dispatch one READ reply per register.
    // Apps must also dump their registers.
    if (DUMP)
    {
        for (uint8_t address = 0; address < CORE_REG_COUNT; ++address)
        {
            send_harp_reply(READ, address);
        }
        self->dump_app_registers();
    }
}

void HarpCore::write_reset_def(msg_t& msg)
{
    uint8_t& write_byte = *((uint8_t*)msg.payload);
    // R_RESET_DEV Register state does not need to be updated since writing to
    // it only triggers behavior.
    // Tease out relevant flags.
    const bool& rst_def_bit = bool((write_byte >> RST_DEF_OFFSET) & 1u);
    // Issue a harp reply only if we aren't resetting.
    // TODO: unclear if this is the appropriate behavior.
    // Reset if specified to do so.
    if (rst_def_bit)
    {
        // Reset core state machine and app.
        self->regs_.r_operation_ctrl_bits.OP_MODE = STANDBY;
        self->reset_app();
    }
    else
        send_harp_reply(WRITE, msg.header.address);
    // TODO: handle the other bit-specific operations.
}

void HarpCore::write_device_name(msg_t& msg)
{
    // TODO.
    // PICO implementation. Write to allocated flash memory
    // since we have no eeprom.
// https://github.com/raspberrypi/pico-examples/blob/master/flash/program/flash_program.c
    write_reg_generic(msg);
}

void HarpCore::write_serial_number(msg_t& msg)
{
    // TODO.
    write_reg_generic(msg);
}

void HarpCore::write_clock_config(msg_t& msg)
{
    // TODO.
    write_reg_generic(msg);
}

void HarpCore::write_timestamp_offset(msg_t& msg)
{
    // TODO.
    write_reg_generic(msg);
}

