#include <pico/stdlib.h>
#include <cstring>
#include <cstdio> // for printf
#include <harp_c_app.h>
#include <harp_synchronizer.h>
#include <core_registers.h>
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
const size_t reg_count = 2;

#pragma pack(push, 1)
struct app_regs_t
{
    volatile uint8_t test_byte;
    volatile uint32_t test_uint;
} app_regs;
#pragma pack(pop)

RegSpecs app_reg_specs[reg_count]
{
    {(uint8_t*)&app_regs.test_byte, sizeof(app_regs.test_byte), U8},
    {(uint8_t*)&app_regs.test_uint, sizeof(app_regs.test_uint), U32}
};


RegFnPair reg_handler_fns[reg_count]
{
    {&HarpCApp::read_app_reg_generic, &HarpCApp::write_app_reg_generic},
    {&HarpCApp::read_app_reg_generic, &HarpCApp::write_app_reg_generic}
};

void update_app_state()
{
    // update here! (Called inside run() function.)
}

// Create Harp App.
HarpCApp& app = HarpCApp::init(who_am_i, hw_version_major, hw_version_minor,
                               assembly_version,
                               harp_version_major, harp_version_minor,
                               fw_version_major, fw_version_minor,
                               serial_number, "Example C App",
                               &app_regs, app_reg_specs,
                               reg_handler_fns, reg_count, update_app_state);

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
