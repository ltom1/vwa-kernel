///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  The Programmable Interrupt Controller driver.
///
/// Contains functions for programming the PIC.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <pic.h>
#include <tty.h>
#include <x86.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Sends an End Of Interrupt signal for an IRQ.
///
/// @param  irq     The IRQ that needs it.
///////////////////////////////////////////////////////////////////////////////////////////////////
void pic_eoi(u8 irq) {
    // also send it to the slave PIC if it came from there
    if (irq >= 8) x86_outb(PIC_SLAVE_CMD, PIC_EOI);

    x86_outb(PIC_MASTER_CMD, PIC_EOI);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the Programmable Interrupt Controller.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pic_init(void) {

    tty_puts(WHITE_ON_BLACK, "Setting up PIC...");

    pic_remap(0x20, 0x28);

    tty_puts(WHITE_ON_BLACK, "Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Reinitializes the PIC with given offsets.
///
/// @param  off_master  The master's offset.
/// @param  off_slave   The slave's offset.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pic_remap(u64 off_master, u64 off_slave) {

    u8 mask_master = x86_inb(PIC_MASTER_DATA);
    u8 mask_slave = x86_inb(PIC_SLAVE_DATA);

    // setup cascading mode
    x86_outb(PIC_MASTER_CMD, ICW1_INIT | ICW1_ICW4);
    x86_io_wait();
    x86_outb(PIC_SLAVE_CMD, ICW1_INIT | ICW1_ICW4);
    x86_io_wait();

    // set vector offsets
    x86_outb(PIC_MASTER_DATA, off_master);
    x86_io_wait();
    x86_outb(PIC_SLAVE_DATA, off_slave);
    x86_io_wait();

    // master PIC: slave PIC exists
    x86_outb(PIC_MASTER_DATA, 4);
    x86_io_wait();
    // slave PIC: cascade identity
    x86_outb(PIC_SLAVE_DATA, 2);
    x86_io_wait();

    // 8086 mode
    x86_outb(PIC_MASTER_DATA, ICW4_8086);
    x86_io_wait();
    x86_outb(PIC_SLAVE_DATA, ICW4_8086);
    x86_io_wait();

    // restore masks
    x86_outb(PIC_MASTER_DATA, mask_master);
    x86_io_wait();
    x86_outb(PIC_SLAVE_DATA, mask_slave);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief   Makes the PIC mask an IRQ.
///
/// @param  irq     The IRQ to mask/disable.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pic_mask_irq(u8 irq) {

    u16 port;
 
    if(irq < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }
    u8 val = x86_inb(port) | (1 << irq);
    x86_outb(port, val);        
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Makes the PIC unmask an IRQ.
///
/// @param  irq     The IRQ to unmask/enable.
///////////////////////////////////////////////////////////////////////////////////////////////////
 
void pic_unmask_irq(u8 irq) {

    u16 port;
 
    if(irq < 8) {
        port = PIC_MASTER_DATA;
    } else {
        port = PIC_SLAVE_DATA;
        irq -= 8;
    }
    u8 val = x86_inb(port) & ~(1 << irq);
    x86_outb(port, val);        
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Disables the PIC.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pic_disable(void) {

    x86_outb(PIC_SLAVE_DATA, PIC_DISABLE);
    x86_outb(PIC_MASTER_DATA, PIC_DISABLE);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Reads a pic register.
///
/// @param  reg     The register to read.
///////////////////////////////////////////////////////////////////////////////////////////////////

u16 pic_get_reg(u64 reg) {

    x86_outb(PIC_MASTER_CMD, reg);
    x86_outb(PIC_SLAVE_CMD, reg);
    return (x86_inb(PIC_SLAVE_CMD) << 8) | x86_inb(PIC_MASTER_CMD);
}
