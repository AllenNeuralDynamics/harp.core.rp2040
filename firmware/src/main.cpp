#include <registers.h>  // do we need this at this level?
#include <harp_core.h>
#include <harp_message.h>
#include <pico/stdlib.h>
#include <cstdio>


// Create device name array.
const uint16_t who_am_i = 1216;
const uint16_t hw_version = 0;
const uint8_t assembly_version = 0;
const uint16_t harp_version = 0;
const uint16_t fw_version = 0;

// Create Registers.
HarpCore core{who_am_i, hw_version, assembly_version, harp_version,
              fw_version};
/*
Registers reg_mem{who_am_i, hw_version, assembly_version, harp_version,
                  fw_version};
*/


// Core0 main.
int main()
{
    stdio_init_all();

    while(true)
    {
        core.regs_.regs_.R_TIMESTAMP_SECOND = 10;  // works.
        // Struct access.
        printf("WHO_AM_I (struct): %d\r\n", core.regs_.regs_.R_TIMESTAMP_SECOND);
        // enum access. Messy, but works.
        printf("WHO_AM_I (ptr): %d\r\n",*((volatile uint32_t*)core.regs_.name2reg[RegNames::TIMESTAMP_SECOND]));
        // Callback trigger by handling a fake incoming message.
        printf("WHO_AM_I (callback):");
        msg_t msg{msg_type_t::READ, 10, RegNames::WHO_AM_I, 10,
                  payload_type_t:: U8};//payload};
        core.handle_message(msg);
        sleep_ms(3000);
    }
    return 0;
}
