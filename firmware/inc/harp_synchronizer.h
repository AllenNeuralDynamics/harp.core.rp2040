#ifndef HARP_SYNCHRONIZER_H
#define HARP_SYNCHRONIZER_H
#include <stdint.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include <hardware/structs/timer.h>

#define HARP_SYNC_BAUDRATE_KBPS (100000)
#define HARP_SYNC_DATA_BITS (8)
#define HARP_SYNC_STOP_BITS (1)
#define HARP_SYNC_PARITY (UART_PARITY_NONE)

extern volatile uint8_t packet_index;
extern volatile uint8_t sync_data[6];
extern volatile uint32_t last_time_us;

// Synchronizer that updates RP2040's timekeeping registers according to
//  specific uart input. Singleton.
class HarpSynchronizer
{
private:
    // Make constructor private.
    HarpSynchronizer(uart_inst_t* uart_id, uint8_t uart_rx_pin);
    ~HarpSynchronizer();
public:
    // Disable default constructor, copy constructor, and assignment operator.
    HarpSynchronizer() = delete;
    HarpSynchronizer(HarpSynchronizer& other) = delete;
    void operator=(const HarpSynchronizer& other) = delete;

/**
 * \brief init the HarpSynchronizer singleton and return a reference to it.
 */
    static HarpSynchronizer& init(uart_inst_t* uart, uint8_t uart_rx_pin);

/**
 * \brief return a pointer to the one-and-only instance.
 * \warning this will fail catastrophically if init was never called.
 */
    static HarpSynchronizer& instance(){return *self;}

private:
    static inline HarpSynchronizer* self = nullptr;

    static void uart_rx_callback();

    uart_inst_t* uart_id_;
    volatile uint8_t packet_index_;
    volatile uint8_t sync_data_[6];
    volatile uint32_t last_char_received_time_us_;
};

#endif // HARP_SYNCHRONIZER_H
