#ifndef HARP_MESSAGE_H
#define HARP_MESSAGE_H

#define IS_SIGNED (0x80)

enum msg_type_t: uint8_t
{
    READ = 1,
    WRITE = 2,
    EVEN = 3,
    READ_ERROR = 9,
    WRITE_ERROR = 10
};

enum payload_type_t: uint8_t
{
    U8 = 1,
    S8 = IS_SIGNED | U8,
    U16 = 2,
    S16 = IS_SIGNED | U16
    // TODO: finish these.
};

// Byte-align struct data so we can memcopy it.
#pragma pack(push, 1)
struct msg_t
{
    msg_type_t msg_type;
    uint8_t length;
    uint8_t address;
    uint8_t port;
    payload_type_t payload_type;
    uint8_t payload[];
};
#pragma pack(pop)

#endif // HARP_MESSAGE_H
