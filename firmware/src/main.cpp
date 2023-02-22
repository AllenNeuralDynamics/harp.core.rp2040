#include <registers.h>
#include <reg_bits.h>
#include <pico/stdlib.h>
#include <cstdio>


// Create device name array.
char device_name[25];

// Create Registers.
RegMemory reg_mem = RegMemory{1216, 0x00, 0x01, 0x02,
                              0x03, 0x04, 0x05, 0x06,
                              0x00000007, 0x0008, 0x09, 0x0a,
                              &device_name, 0xCAFE, 0x0c, 0x0d};


// Core0 main.
int main()
{
    stdio_init_all();

    while(true)
    {
        printf("sizeof reg_mem in bytes: %d.\r\n",sizeof(reg_mem));
        printf("----\r\n");
        printf("Device ID: 0x%x\r\n", reg_mem.names.R_WHO_AM_I);
        printf("VERSION_H: 0x%x\r\n", reg_mem.names.R_HW_VERSION_H);
        printf("VERSION_L: 0x%x\r\n", reg_mem.names.R_HW_VERSION_L);
        printf("ASSEMBLY_VERSION: 0x%x\r\n", reg_mem.names.R_ASSEMBLY_VERSION);
        printf("SERIAL NUM: (hex) 0x%x\r\n", reg_mem.names.R_SERIAL_NUMBER);
        printf("----\r\n");
        printf("reg mem: ");
        //for (auto i = 0; i < 16; ++i)
        //    printf("%x ",reg_mem.mem[i]);
        printf("Device ID: 0x%x\r\n", reg_mem.mem[RegNames::WHO_AM_I]);
        printf("VERSION_H: 0x%x\r\n", reg_mem.mem[RegNames::HW_VERSION_H]);
        printf("VERSION_L: 0x%x\r\n", reg_mem.mem[RegNames::HW_VERSION_L]);
        printf("ASSEMBLY_VERSION: 0x%x\r\n", reg_mem.mem[RegNames::ASSEMBLY_VERSION]);
        printf("SERIAL NUM: (hex) 0x%x\r\n", reg_mem.mem[RegNames::SERIAL_NUMBER]);
        printf("\r\n\r\n");
        sleep_ms(3000);
    }
    return 0;
}
