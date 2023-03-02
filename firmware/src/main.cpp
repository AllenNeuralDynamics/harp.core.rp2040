#include <harp_core.h>
#include <harp_message.h>
#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>
#include <tusb.h>

// Create device name array.
const uint16_t who_am_i = 1216;
const uint16_t hw_version = 0;
const uint8_t assembly_version = 0;
const uint16_t harp_version = 0;
const uint16_t fw_version = 0;

// Create Registers.
HarpCore& core = HarpCore::init(who_am_i, hw_version, assembly_version,
                                harp_version, fw_version);
//DeviceCore& harp_dev = DevCore::init(who_am_i, hw_version, assembly_version,
//                                     harp_version, fw_version);
// Under the hood, should init:
//  * DMA UART to TX
//  * SYSTICK to update at 1 [us] intervals.
//  * SYSTICK IRQ to update TIMESTAMP_SECONDS.
//  * synchronizer interrupt handler?

// TODO:
//  * DONE parse messages.
//  * DONE dispatch messages to the appropriate callback function.
//  * DONE Handle replying to read requests.
//  * Handle basic timekeeping/updating from Harp Message.
//  * Handle basic timekeeping/updating from clock synchronizer.

// Overwrite weak reference to systick IRQ in crt0.s.
//extern void isr_systick()
//{
//    tud_task();  // happens at 1[ms] intervals.
//}

//void cdc_write(uint8_t buf[], uint8_t num_bytes)
//{
//    for (uint8_t i = 0; i < num_bytes; ++i)
//        tud_cdc_write_char(buf[i]);
//    tud_cdc_write_flush();
//}

// Core0 main.
int main()
{
    //stdio_init_all();
    stdio_usb_init();
    //stdio_uart_init();

    // Note: tud_cdc_connected checks DTR. tud_ready does not.
    //while (!tud_ready()){} // Block until connection to serial port.
    while (!stdio_usb_connected()){} // Block until connection to serial port.

    while(true)
    {
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
        uint8_t buf[] = {'H', 'e', 'l', 'l', 'o', '\r', '\n'};
        for (auto i = 0; i < sizeof(buf); ++i)
            tud_cdc_write_char(buf[i]);
        tud_cdc_write_flush();
        sleep_ms(3000);
    }
    return 0;
}
