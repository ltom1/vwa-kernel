///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file 
/// @brief  Contains important type definitions
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


typedef unsigned    char        u8;
typedef signed      char        s8;
typedef unsigned    short       u16;
typedef signed      short       s16;
typedef unsigned    int         u32;
typedef signed      int         s32;
typedef unsigned    long long   u64;
typedef signed      long long   s64;

#define NORETURN    __attribute((noreturn))
#define PACKED      __attribute((packed))
#define INLINE      inline __attribute((always_inline))
#define ALIGNED(n)  __attribute((aligned(n)))

#define bool        _Bool
#define true        1
#define false       0

#define QWORD(high_dword, low_dword)    (((u64)(high_dword) << 32) + (u64)(low_dword))
#define DWORD(high_word, low_word)      (((u32)(high_word) << 16) + (u64)(low_word))
#define WORD(high_byte, low_byte)       (((u16)(high_byte) << 8) + (u64)(low_byte))
#define HIGH_BYTE(value)                ((value >> 8) & 0xFF)
#define LOW_BYTE(value)                 (value & 0xFF)
