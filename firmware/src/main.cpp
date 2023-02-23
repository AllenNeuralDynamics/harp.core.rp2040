#include <registers.h>
#include <pico/stdlib.h>
#include <cstdio>


// Create device name array.
const uint16_t who_am_i = 1216;
const uint16_t hw_version = 0;
const uint8_t assembly_version = 0;
const uint16_t harp_version = 0;
const uint16_t fw_version = 0;

// Create Registers.
/*
RegMemory reg_mem = RegMemory{1216, 0x00, 0x01, 0x02,
                              0x03, 0x04, 0x05, 0x06,
                              0x00000007, 0x0008, 0x09, 0x0a,
                              &device_name, 0xCAFE, 0x0c, 0x0d};
*/
Registers reg_mem{who_am_i, hw_version, assembly_version, harp_version,
                  fw_version};


// Core0 main.
int main()
{
    stdio_init_all();

    while(true)
    {
        reg_mem.regs.R_TIMESTAMP_SECOND = 10;  // works.
        // Struct access.
        printf("WHO_AM_I (struct): %d\r\n",reg_mem.regs.R_TIMESTAMP_SECOND);
        // enum access. Messy, but works.
        printf("WHO_AM_I (ptr): %d\r\n",*((volatile uint32_t*)reg_mem.name2reg[RegNames::TIMESTAMP_SECOND]));
        sleep_ms(3000);
    }
    return 0;
}
