#ifndef REG_TYPES_H
#define REG_TYPES_H

#include <stdint.h>

// Payload flags. These need their type enforced.
#define IS_SIGNED ((uint8_t)0x80)
#define IS_FLOAT ((uint8_t)0x40)
#define HAS_TIMESTAMP ((uint8_t)0x10)


enum class RegType: uint8_t
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

#endif // REG_TYPES_H
