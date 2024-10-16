///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains IDT related stuff.
///
/// Contains function definitions for loading and editing of the Interrupt Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <idt.h>
#include <gdt.h>
#include <paging.h>
#include <tty.h>


/// @brief  The Interrupt Descriptor Table (IDT) as a global var.
///
/// Used to handle different software and hardware interrupts.
ALIGNED(PAGE_SIZE) idt_desc_t idt[IDT_ENTRIES];
/// @brief  The Interrupt Descriptor Table Register (IDTR) as a global var.
///
/// Used for loading an IDT.
/// Loaded by a lidt instruction.
static idtr_t idtr = {0};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the Interrupt Descriptor Table.
///////////////////////////////////////////////////////////////////////////////////////////////////

void idt_init(void) {

    tty_puts(WHITE_ON_BLACK, "Setting up IDT...");

    idtr.base = (u64)&idt[0];
    idtr.limit = (u16)sizeof(idt_desc_t) * IDT_ENTRIES - 1;

    idt_load(&idtr);

    tty_puts(WHITE_ON_BLACK, "Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Sets a descriptor in the Interrupt Descriptor Table.
///
/// @param  vec     The interrupt number/vector/offset into the IDT.
/// @param  isr     The address of the handler funtion.
/// @param  attr    The descriptor's attributes.
/// @param  ist     The Interrupt stack table number.
///////////////////////////////////////////////////////////////////////////////////////////////////

void idt_set_desc(u8 vec, u64 isr, u8 attr, u8 ist) {

    idt_desc_t *desc = &idt[vec];

    desc->base_low = isr & 0xFFFF;
    desc->cs = KERNEL_CODE;
    desc->ist = ist;
    desc->attr = attr;
    desc->base_mid = (isr >> 16) & 0xFFFF;
    desc->base_high = (isr >> 32) & 0xFFFFFFFF;
    desc->zero = 0;
}
