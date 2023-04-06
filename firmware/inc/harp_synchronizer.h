#ifndef HARP_SYNCHRONIZER_H
#define HARP_SYNCHRONIZER_H
#include <stdint.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include <hardware/structs/timer.h>
#include <cstring> // for memcpy

#ifdef DEBUG
#include <cstdio> // for printf
#endif

#define HARP_SYNC_BAUDRATE (100000)
#define HARP_SYNC_DATA_BITS (8)
#define HARP_SYNC_STOP_BITS (1)
#define HARP_SYNC_PARITY (UART_PARITY_NONE)

#define HARP_SYNC_OFFSET_US (672 - 90) // time (in [us]) from the **end** of
                                       // the last packet byte and the time
                                       // specified in that packet.

// Synchronizer that updates RP2040's timekeeping registers according to
//  specific uart input. Singleton.
class HarpSynchronizer
{
public:
    enum SyncState
    {
        RECEIVE_HEADER_0,
        RECEIVE_HEADER_1,
        RECEIVE_TIMESTAMP
    };

private:
    // Make constructor/destructor private.
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
 * \brief return a pointer to the one-and-only instance or nullptr if
 *      init() was never called.
 */
    static HarpSynchronizer& instance(){return *self;}

private:
/**
 * \brief a pointer to the one-and-only instance or nullptr if init() was
 *      never called.
 */
    static inline HarpSynchronizer* self = nullptr;

/**
 * \brief Callback fn for uart interrupt triggered when a new byte arrives.
 * \note Interrupt callbacks must be static, so this fn refers use the self ptr
 *      to access the singleton data members.
 */
    static void uart_rx_callback();

    uart_inst_t* uart_id_;

    volatile SyncState state_;
    volatile uint8_t packet_index_;
    volatile bool new_timestamp_;
/**
 * \brief container to store the little-endian timestamp and then
 *  reinterpret-cast to the value.
 */
    alignas(uint32_t) volatile uint8_t sync_data_[4];
};

#endif // HARP_SYNCHRONIZER_H
