#ifndef HARP_MESSAGE_H
#define HARP_MESSAGE_H

// Payload flags. These need their type enforced.
#define IS_SIGNED ((uint8_t)0x80)
#define IS_FLOAT ((uint8_t)0x40)
#define HAS_TIMESTAMP ((uint8_t)0x10)

#define MAX_PACKET_SIZE (255) // unused?

enum msg_type_t: uint8_t
{
    READ = 1,
    WRITE = 2,
    EVENT = 3,
    READ_ERROR = 9,
    WRITE_ERROR = 10
};

enum payload_type_t: uint8_t
{
    U8 = 1,
    S8 = IS_SIGNED | U8,
    U16 = 2,
    S16 = IS_SIGNED | 2, // 130
    U32 = 4,
    S32 = IS_SIGNED | 4,
    U64 = 8,
    S64 = IS_SIGNED | 8,
    Float = IS_FLOAT | 4,
    Timestamp = HAS_TIMESTAMP,
    TimestampedU8 = HAS_TIMESTAMP | U8,
    TimestampedS8 = HAS_TIMESTAMP | S8,
    TimestampedU16 = HAS_TIMESTAMP | U16,
    TimestampedS16 = HAS_TIMESTAMP | S16,
    TimestampedU32 = HAS_TIMESTAMP | U32,
    TimestampedS32 = HAS_TIMESTAMP | S32,
    TimestampedU64 = HAS_TIMESTAMP | U64,
    TimestampedS64 = HAS_TIMESTAMP | S64,
    TimestampedFloat = HAS_TIMESTAMP | Float
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
    payload_type_t payload_type;

    bool has_timestamp() {return bool((payload_type & HAS_TIMESTAMP) >> 4);}
    uint8_t payload_length() {return has_timestamp()? raw_length - 10: raw_length - 4;}
    uint8_t payload_base_index_offset() {return has_timestamp()? 11: 5;}
    uint8_t checksum_index_offset(){return 2 + raw_length;}
};
#pragma pack(pop)

// Reference-only convenience classes.
// The data needs to exist elsewhere (i.e: in the RX buffer.
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
    // Member functions:
    bool has_timestamp() {return header.has_timestamp();}
    uint8_t payload_length() {return header.payload_length();}
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
