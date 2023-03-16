#include <harp_synchronizer.h>

// RX interrupt handler

HarpSynchronizer::HarpSynchronizer(uart_inst_t* uart_id, uint8_t uart_rx_pin)
:uart_id_{uart_id}, packet_index_{0}, sync_data_{0, 0, 0, 0, 0, 0},
 last_char_received_time_us_{time_us_32()}
{
    // Create a pointer to the first (and one-and-only) instance created.
    if (self == nullptr)
        self = this;
    // Setup uart.
    uart_init(uart_id_, HARP_SYNC_BAUDRATE_KBPS);
    // Disable hardware flow control.
    uart_set_hw_flow(uart_id_, false, false);
    // Set data format
    uart_set_format(uart_id_, HARP_SYNC_DATA_BITS, HARP_SYNC_STOP_BITS,
                    HARP_SYNC_PARITY);
    // Setup the RX pin by using the function select on the GPIO
    gpio_set_function(uart_rx_pin, GPIO_FUNC_UART);
    // Turn off internal FIFO. (FIFO set to size 1.)
    uart_set_fifo_enabled(uart_id, false);
    // Set up an RX interrupt and attach the callback function.
    // Select correct interrupt handler for the UART we are using
    int UART_IRQ = (uart_id == uart0) ? UART0_IRQ : UART1_IRQ;
    irq_set_exclusive_handler(UART_IRQ, uart_rx_callback);
    irq_set_enabled(UART_IRQ, true);
    // Enable RX-based interrupts.
    uart_set_irq_enables(uart_id_, true, false);
}

HarpSynchronizer::~HarpSynchronizer(){self = nullptr;}

HarpSynchronizer& HarpSynchronizer::init(uart_inst_t* uart_id,
                                         uint8_t uart_rx_pin)
{
    static HarpSynchronizer synchronizer(uart_id, uart_rx_pin);
    return synchronizer;
}

void HarpSynchronizer::uart_rx_callback()
{
    // Update seconds and microseconds to be current time
    // (i.e:  572 usec before the specified second.)
    while (uart_is_readable(self->uart_id_))
    {
        // Reset packet index if it's been a while since we received data.
        uint32_t curr_time_us = time_us_32();
        if (curr_time_us - self->last_char_received_time_us_ > 500)
        {
            self->packet_index_ = 0;
        }
        self->last_char_received_time_us_ = curr_time_us;
        // Read the incoming byte.
        self->sync_data_[self->packet_index_++] = uart_getc(self->uart_id_);
        if (self->packet_index_ == 6);
        {
            // Update timestamp register seconds and microseconds.
            if (self->sync_data_[0] == 0xAA && self->sync_data_[1] == 0xAF)
            {
                // Interpret 4-byte sequence from index 2 onwards as a
                // little-endian uint32_t.
                uint64_t curr_us = uint64_t(*((uint32_t*)(&self->sync_data_[2]))) * 1000000 - 572;
                timer_hw->timelw = (uint32_t)curr_us;
                timer_hw->timehw = (uint32_t)(curr_us >> 32);
            }
            // Reset for the next packet.
            self->packet_index_ = 0;
            self->sync_data_[0] = 0;
            self->sync_data_[1] = 0;
        }
    }
}

