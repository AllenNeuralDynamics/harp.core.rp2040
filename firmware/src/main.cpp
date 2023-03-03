#include <harp_core.h>
#include <harp_message.h>
#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>
#include <tusb.h>

// Create device name array.
const uint16_t who_am_i = 1216;
const uint16_t hw_version = 1;
const uint8_t assembly_version = 2;
const uint16_t harp_version = 3;
const uint16_t fw_version = 4;

// Create Registers.
HarpCore& core = HarpCore::init(who_am_i, hw_version, assembly_version,
                                harp_version, fw_version, "Pico Harp");
// Specific device implementations will inherit from HarpCore and get
// instantiated similarly:
//DeviceCore& harp_dev = DevCore::init(who_am_i, hw_version, assembly_version,
//                                     harp_version, fw_version);

// TODO:
//  * DONE parse messages.
//  * DONE dispatch messages to the appropriate callback function.
//  * DONE Handle replying to read requests.
//  * DONE time keeping/updating from Harp Message SECONDS and MICROSECONDS register writes.
//  * DONE populating reg name field on initialization.
//  * DONE parse harp messages
//  * DONE send harp replies
//  * Handle time keeping/updating from clock synchronizer.
// Extra: shouldn't need this since we should be able to handle requests one at a time.
//  * queue incoming harp messages.
//  * queue outgoing harp messages.

// Core0 main.
int main()
{
    stdio_usb_init();
    //stdio_uart_init();

    while (!stdio_usb_connected()){} // Block until connected to serial port.

    while(true)
    {
        core.handle_rx_buffer_input();  // entry point. Could be renamed run()
    /*
        core.regs.R_TIMESTAMP_SECOND = 10;

        // Struct access.
        printf("WHO_AM_I (struct): %d\r\n", core.regs.R_TIMESTAMP_SECOND);

        // Callback trigger by handling a fake message.
        printf("WHO_AM_I (callback):");
        uint8_t payload[] = {1, 2, 3};
        uint8_t rx_buffer_data[] = {msg_type_t::READ, 7, RegNames::WHO_AM_I, 255,
                                    payload_type_t::U8, payload[0], payload[1], payload[2], 0};
        memcpy(core.rx_buffer_, rx_buffer_data, 9);
        core.handle_rx_buffer_message();
*/
        //core_.read_timestamp_microsecond();

// Test write to usb packet directly and dispatch it.
/*
        uint8_t buf[] = {'H', 'e', 'l', 'l', 'o', '\r', '\n'};
        for (auto i = 0; i < sizeof(buf); ++i)
            tud_cdc_write_char(buf[i]);
        tud_cdc_write_flush();
*/
//        sleep_ms(3000);
    }
    return 0;
}
