#include <harp_synchronizer.h>


HarpSynchronizer::HarpSynchronizer(uart_inst_t* uart_id, uint8_t uart_rx_pin)
:uart_id_{uart_id}, packet_index_{0}, sync_data_{0, 0, 0, 0, 0, 0},
 state_{RECEIVE_HEADER}, last_char_received_time_us_{time_us_32()}
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
    SyncState next_state_{state_};  // Init next state with curr state value.
    uint8_t new_byte;
    // This State machine "ticks" every time we receive at least one new byte.
    // Handle next-state logic.
    while (uart_is_readable(self->uart_id_))
    {
        new_byte = uart_getc(self->uart_id_);
        switch (state_)
        {
            case RECEIVE_HEADER_0:
                if (new_byte == 0xAA)
                    next_state_ = RECEIVE_HEADER_1;
                break;
            case RECEIVE_HEADER_1:
                if (new_byte == 0xAF)
                    next_state_ = RECEIVE_TIMESTAMP;
                else
                    next_state_ = RECEIVE_HEADER_0;
                break;
            case RECEIVE_TIMESTAMP:
                sync_data_[packet_index_++] = new_byte;
                if (packet_index_ == 4)
                    new_timestamp_ = true;
                break;
        }
        state_ = next_state_;
    }
    if (not new_timestamp_)
        return;
    // Apply new timestamp data.
    // Interpret 4-byte sequence from index 2 onwards as a
    // little-endian uint32_t.
    uint64_t curr_us = uint64_t(*((uint32_t*)(&self->sync_data_[2]))) * 1000000
                       - HARP_SYNC_OFFSET_US;
    timer_hw->timelw = (uint32_t)curr_us;
    timer_hw->timehw = (uint32_t)(curr_us >> 32);
    // Cleanup
    packet_index_ = 0;
    new_timestamp_ = false;
}

