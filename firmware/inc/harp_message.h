#ifndef HARP_MESSAGE_H
#define HARP_MESSAGE_H
#include "reg_types.h"
#include <stdint.h>

#define MAX_PACKET_SIZE (255) // unused?

enum msg_type_t: uint8_t
{
    READ = 1,
    WRITE = 2,
    EVENT = 3,
    READ_ERROR = 9,
    WRITE_ERROR = 10
};


// Byte-align struct data so we can either:
// memcopy it to struct or cast the rx buffer to struct
#pragma pack(push, 1)
struct msg_header_t
{
    msg_type_t type;
    uint8_t raw_length;
    uint8_t address;
    uint8_t port; // should default to 255.
    RegType payload_type;

    // (Inline) Member functions:
    bool has_timestamp()
    {return bool(payload_type & HAS_TIMESTAMP);}

    uint8_t payload_length()
    {return has_timestamp()? raw_length - 10: raw_length - 4;}

    uint8_t payload_base_index_offset()
    {return has_timestamp()? 11: 5;}

    uint8_t checksum_index_offset()
    {return 2 + raw_length;}

    uint8_t msg_size()
    {return raw_length + 2;}
};
#pragma pack(pop)

// Reference-only convenience classes.
// The data needs to exist elsewhere (i.e: in the RX buffer).
struct msg_t
{
    msg_header_t& header;
    void* payload;  // unknown type until we parse the header.
    uint8_t& checksum;

    // Custom reference-only constructor that refers to data in existing
    // memory locations.
    msg_t(msg_header_t& header, void* payload, uint8_t& checksum)
        :header{header}, payload{payload}, checksum{checksum}
    {}

    // (Inline) Member functions:
    bool has_timestamp()
    {return header.has_timestamp();}

    uint8_t payload_length()
    {return header.payload_length();}
};

struct timestamped_msg_t: public msg_t
{
    uint32_t& timestamp_sec;
    uint16_t& timestamp_usec;

    // Custom reference-only constructor that refers to data in existing
    // memory locations.
    timestamped_msg_t(msg_header_t& header, uint32_t& timestamp_sec,
                      uint16_t& timestamp_usec, void* payload,
                      uint8_t& checksum)
    : msg_t(header, payload, checksum),
    timestamp_sec{timestamp_sec}, timestamp_usec{timestamp_usec}
    {}
};

#endif // HARP_MESSAGE_H
