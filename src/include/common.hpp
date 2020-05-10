#pragma once
#include <cstdint>

#define NTH_BIT(x, n) (((x) >> (n)) & 1)

/* Integer type shortcuts */
typedef uint8_t  u8;  typedef int8_t  s8;
typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32;
typedef uint64_t u64; typedef int64_t s64;
