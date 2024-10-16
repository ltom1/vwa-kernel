///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  PIC driver header file.
///
/// Contains declarations and type definitions. 
/// Used when programming the Progammable Interrupt Controller.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define PIC_MASTER          0x20
#define PIC_MASTER_CMD	    PIC_MASTER
#define PIC_MASTER_DATA	    (PIC_MASTER + 1)

#define PIC_SLAVE           0xA0
#define PIC_SLAVE_CMD	    PIC_SLAVE
#define PIC_SLAVE_DATA	    (PIC_SLAVE + 1)

#define PIC_EOI		        0x20
#define PIC_DISABLE		    0xFF

#define ICW1_ICW4	        0x01
#define ICW1_SINGLE	        0x02
#define ICW1_INTERVAL4	    0x04
#define ICW1_LEVEL	        0x08
#define ICW1_INIT	        0x10
 
#define ICW4_8086	        0x01
#define ICW4_AUTO	        0x02
#define ICW4_BUF_SLAVE	    0x08
#define ICW4_BUF_MASTER	    0x0C
#define ICW4_SFNM	        0x10

#define PIC_REG_IRR         0x0a
#define PIC_REG_ISR         0x0b


void pic_init(void);
void pic_remap(u64 off_master, u64 off_slave);
void pic_disable(void);
void pic_eoi(u8 irq);
void pic_mask_irq(u8 irq);
void pic_unmask_irq(u8 irq);
u16 pic_get_reg(u64 reg);
