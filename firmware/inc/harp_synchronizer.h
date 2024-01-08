#ifndef HARP_SYNCHRONIZER_H
#define HARP_SYNCHRONIZER_H
#include <stdint.h>
#include <pico/stdlib.h>
#include <hardware/uart.h>
#include <hardware/irq.h>
#include <hardware/sync.h>
#include <hardware/structs/timer.h>

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

/**
 * \brief get the total elapsed microseconds (64-bit) in "Harp" time.
 * \warning this value is not monotonic and can change at any time if an
 *  external synchronizer is physically connected and operating.
 */
    static uint64_t time_us_64()
    {return ::time_us_64() - self->offset_us_64_;}

/**
 * \brief get the total elapsed microseconds (32-bit) in "Harp" time.
 * \warning this value is not monotonic and can change at any time if an
 *  external synchronizer is physically connected and operating.
 */
    static uint32_t time_us_32()
    {return ::time_us_32() - uint32_t(self->offset_us_64_);}

/**
 * \brief convert harp time (in 64-bit microseconds) to local system time
 *  (in 64-bit microseconds).
 * \details this utility function is useful for setting alarms in the device's
 *  local time domain, which is monotonic and unchanged by adjustments to
 *  the harp time.
 */
    static uint64_t harp_to_system_us_64(uint64_t harp_time_us)
    {return harp_time_us + self->offset_us_64_;}

/**
 * \brief convert harp time (in 32-bit microseconds) to local system time
 *  (in 32-bit microseconds).
 * \details this utility function is useful for setting alarms in the device's
 *  local time domain, which is monotonic and unchanged by adjustments to
 *  the harp time.
 */
    static uint32_t harp_to_system_us_32(uint32_t harp_time_us)
    {return harp_time_us + uint32_t(self->offset_us_64_);}

/**
 * \brief true if the synchronizer has received at least one external sync
 *  signal.
 */
    static bool has_synced()
    {return self->has_synced_;}

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

    uint64_t offset_us_64_;

    volatile bool has_synced_;
/**
 * \brief container to store the little-endian timestamp and then
 *  reinterpret-cast to the value.
 */
    alignas(uint32_t) volatile uint8_t sync_data_[4];

/**
 * \brief HarpCore is a friend such that updating the HarpCore's timestamp
 *  registers will update the HarpSynchronizer #offset_us_64_ instead of
 *  the HarpCore's internal offset.
 */
    friend class HarpCore;
};

#endif // HARP_SYNCHRONIZER_H
