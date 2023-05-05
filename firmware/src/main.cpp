#include <harp_core.h>
#include <harp_synchronizer.h>
#include <harp_message.h>
#include <pico/stdlib.h>
#include <cstring>

#ifdef DEBUG
#include <cstdio>
#endif


// Create device name array.
const uint16_t who_am_i = 1216;
const uint8_t hw_version_major = 1;
const uint8_t hw_version_minor = 0;
const uint8_t assembly_version = 2;
const uint8_t harp_version_major = 2;
const uint8_t harp_version_minor = 0;
const uint8_t fw_version_major = 3;
const uint8_t fw_version_minor = 0;
const uint16_t serial_number = 1234;

// Create Core and synchronizer.
HarpCore& core = HarpCore::init(who_am_i, hw_version_major, hw_version_minor,
                                assembly_version,
                                harp_version_major, harp_version_minor,
                                fw_version_major, fw_version_minor,
                                serial_number, "Pico Harp");

// Core0 main.
int main()
{

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
#ifdef DEBUG
    stdio_uart_init_full(uart1, 921600, 4, -1); // use uart1 tx only.
    printf("Hello, from a Pi Pico!\r\n");
#endif
    // TODO: can this happen outside of main if we are not debugging?
    HarpSynchronizer& sync = HarpSynchronizer::init(uart0, 17);
    while(true)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, !gpio_get(PICO_DEFAULT_LED_PIN));
        core.run(); // call this in a loop.
        // run() will:
        // 1. parse new messages into a buffer.
        // 2. Handle register reads and write messages to core registers.
        // 3. Reply with the appropriate harp reply for reads/writes to core
        //    registers.
        // Optional. Handle msgs outside the range of the core registers here.
        if (not core.new_msg())
            continue;
        // Handle the message here.
        core.clear_msg();
    }
    return 0;
}
