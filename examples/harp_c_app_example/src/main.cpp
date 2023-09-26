#include <pico/stdlib.h>
#include <cstring>
#include <cstdio> // for printf
#include <config.h>
#include <harp_core.h>
#include <harp_synchronizer.h>
#include <registers.h>
#include <reg_types.h>

// Create device name array.
const uint16_t who_am_i = 1234;
const uint8_t hw_version_major = 1;
const uint8_t hw_version_minor = 0;
const uint8_t assembly_version = 2;
const uint8_t harp_version_major = 2;
const uint8_t harp_version_minor = 0;
const uint8_t fw_version_major = 3;
const uint8_t fw_version_minor = 0;
const uint16_t serial_number = 0xCAFE;

// Harp App Register Setup.
const reg_count = 2;

struct app_regs_t
{
    volatile uint8_t test_byte;
    volatile uint32_t test_uint;
} app_regs;

RegSpecs app_reg_specs[]
{
    {&app_registers.test_byte, sizeof(app_registers.test_byte), U8},
    {&app_registers.test_uint, sizeof(app_registers.test_uint), U32}
};

RegFnPair reg_handler_fns[]
{
    {HarpCApp::read_reg_generic, HarpCApp::write_reg_generic},
    {HarpCApp::read_reg_generic, HarpCApp::write_reg_generic}
};

// Create Harp App.
HarpCApp& app = HarpCApp::init(who_am_i, hw_version_major, hw_version_minor,
                               assembly_version,
                               harp_version_major, harp_version_minor,
                               fw_version_major, fw_version_minor,
                               serial_number, "Example C App",
                               &app_regs, &app_reg_specs,
                               &reg_handler_fns, reg_count);

// Core0 main.
int main()
{
// Init Synchronizer.
    HarpSynchronizer& sync = HarpSynchronizer::init(uart0, 17);
#ifdef DEBUG
    stdio_uart_init_full(uart1, 921600, 4, -1); // use uart1 tx only.
    printf("Hello, from a Pi Pico!\r\n");
#endif
    while(true)
    {
        app.run();
    }
}
