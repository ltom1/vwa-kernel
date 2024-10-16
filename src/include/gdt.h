///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file 
/// @brief  Contains all things related to the Global Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define PRIVILEGE_USER          0x03
#define PRIVILEGE_KERNEL        0x00

#define ACCESS_KERNEL           0x90
#define ACCESS_USER             0xF0
#define ACCESS_CODE             0x0A
#define ACCESS_DATA             0x02
#define ACCESS_TSS              0x89

#define LONG_MODE               (1 << 1)
#define LIMIT_IN_PAGES          (1 << 3)

#define GDT_ENTRIES             6

#define KERNEL_CODE             0x08
#define KERNEL_DATA             0x10
#define USER_CODE               0x18
#define USER_DATA               0x20
#define TSS_DESC                0x28

#define PL_KERNEL               0x0
#define PL_USER                 0x3


/// @brief  Structure of a GDT entry.
typedef struct PACKED ALIGNED(4) GDTEntry {
    u16     limit_low;
    u16     base_low;
    u8      base_mid;
    u8      access;
    u8      limit_high   : 4;
    u8      flags        : 4;
    u8      base_high;
} gdt_entry_t;

/// @brief  Structure of the Task State Segment.
typedef struct PACKED TSS {
    u32     reserved1;
    u64     rsp0;
    u64     rsp1;
    u64     rsp2;
    u64     reserved2;
    u64     ist1;
    u64     ist2;
    u64     ist3;
    u64     ist4;
    u64     ist5;
    u64     ist6;
    u64     ist7;
    u64     reserved3;
    u16     reserved4;
    u16     iopb;
} tss_t;

/// @brief  Structure of the GDT Descriptor.
typedef struct PACKED ALIGNED(4) GDTRegister {
    u16     limit;
    u64     base;
} gdtr_t;


void gdt_init(void);
void gdt_build(void);
gdt_entry_t gdt_create_entry(u64 base, u64 limit, u8 access, u8 flags);


void gdt_load(gdtr_t *gdtr);
void tss_load(u16 tss_desc);

extern tss_t tss;
