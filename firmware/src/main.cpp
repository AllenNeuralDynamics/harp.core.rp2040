#include <registers.h>  // do we need this at this level?
#include <harp_core.h>
#include <harp_message.h>
#include <pico/stdlib.h>
#include <cstdio>
#include <cstring>


// Create device name array.
const uint16_t who_am_i = 1216;
const uint16_t hw_version = 0;
const uint8_t assembly_version = 0;
const uint16_t harp_version = 0;
const uint16_t fw_version = 0;

// Create Registers.
HarpCore core{who_am_i, hw_version, assembly_version, harp_version, fw_version};

// TODO:
//  * DONE parse messages.
//  * DONE dispatch messages to the appropriate callback function.
//  * DONE Handle replying to read requests.
//  * Handle basic timekeeping/updating from Harp Message.
//  * Handle basic timekeeping/updating from clock synchronizer.

// Core0 main.
int main()
{
    stdio_init_all();
    while (!stdio_usb_connected()){} // Block until connection to serial port.

    while(true)
    {
        core.regs_.regs_.R_TIMESTAMP_SECOND = 10;  // works.
        // Struct access.
        printf("WHO_AM_I (struct): %d\r\n", core.regs_.regs_.R_TIMESTAMP_SECOND);
        // enum access. Messy, but works.
        printf("WHO_AM_I (ptr): %d\r\n",
                *((volatile uint32_t*)core.regs_.enum_to_reg_specs[RegNames::TIMESTAMP_SECOND].base_ptr));

        // Callback trigger by handling a fake message.
        printf("WHO_AM_I (callback):");
        uint8_t payload[] = {1, 2, 3};
        uint8_t rx_buffer_data[] = {msg_type_t::READ, 7, RegNames::WHO_AM_I, 255,
                                    payload_type_t::U8, payload[0], payload[1], payload[2], 0};
        memcpy(core.rx_buffer_, rx_buffer_data, 9);
        core.handle_rx_buffer_message();
        sleep_ms(3000);
    }
    return 0;
}
