///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file 
/// @brief  Contains all things related to the Interrupt Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define IDT_ENTRIES             256
#define IDT_USER_ACCESS         (3 << 5)
#define IDT_PRESENT             1
#define IDT_INT_GATE            0x8E
#define IDT_SYSCALL             0xEE


/// @brief  Structure of an IDT entry.
typedef struct PACKED IDTDescriptor {
    u16 base_low;
    u16 cs;
    u8 ist;
    u8 attr;
    u16 base_mid;
    u32 base_high;
    u32 zero;
} idt_desc_t;

/// @brief  Structure of the IDT Descriptor.
typedef struct PACKED IDTRegister {
    u16 limit;
    u64 base;
} idtr_t;


void idt_init(void);
void idt_set_desc(u8 vec, u64 isr, u8 attr, u8 ist);

void idt_load(idtr_t *idtr);
