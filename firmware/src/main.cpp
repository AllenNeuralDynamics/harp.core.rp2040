#include <harp_core.h>
#include <harp_synchronizer.h>
#include <harp_message.h>
#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>
#include <tusb.h>

// Create device name array.
const uint16_t who_am_i = 1216;
const uint8_t hw_version_major = 1;
const uint8_t hw_version_minor = 0;
const uint8_t assembly_version = 2;
const uint8_t harp_version_major = 2;
const uint8_t harp_version_minor = 0;
const uint8_t fw_version_major = 3;
const uint8_t fw_version_minor = 0;

// Create Core and synchronizer.
HarpCore& core = HarpCore::init(who_am_i, hw_version_major, hw_version_minor,
                                assembly_version,
                                harp_version_major, harp_version_minor,
                                fw_version_major, fw_version_minor,
                                "Pico Harp");
HarpSynchronizer& synchro = HarpSynchronizer::init(uart0, 1);

// Core0 main.
int main()
{
    stdio_usb_init();
    //stdio_uart_init();

    while (!stdio_usb_connected()){} // Block until connected to serial port.

    while(true)
    {
        core.run();
    /*
        core.regs.R_TIMESTAMP_SECOND = 10;

        // Struct access.
        printf("WHO_AM_I (struct): %d\r\n", core.regs.R_TIMESTAMP_SECOND);

        // Callback trigger by handling a fake message.
        printf("WHO_AM_I (callback):");
        uint8_t payload[] = {1, 2, 3};
        uint8_t rx_buffer_data[] = {msg_type_t::READ, 7, RegNames::WHO_AM_I, 255,
                                    reg_type_t::U8, payload[0], payload[1], payload[2], 0};
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
