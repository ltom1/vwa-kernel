///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for handling hardware Interrupt Requests.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <idt.h>
#include <isr.h>
#include <irq.h>
#include <tty.h>
#include <pic.h>
#include <err.h>


/// @brief  Array of the handlers for each IRQ.
isr_t irq_handlers[MAX_IRQ];


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Handles hardware Interrupt Requests.
///
/// @param  args    The Trapframe passed on by the original ISR.
///////////////////////////////////////////////////////////////////////////////////////////////////

void irq_handler(int_args_t *args) {

    // check if irq has a registered handler
    isr_t handler = irq_handlers[args->int_vec - MAX_ERR];
    if (handler != 0) handler(args);
    else tty_putf(WHITE_ON_BLACK, 
            "Unregistered IRQ: %u (int %x)\n", args->int_vec - MAX_ERR, args->int_vec);

    pic_eoi(args->int_vec);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Registers an irq handler.
///
/// @param  irq     The Interrupt Number.
/// @param  handler The address of the handler function.
///////////////////////////////////////////////////////////////////////////////////////////////////

void irq_add(u8 irq, isr_t handler) {

    irq_handlers[irq] = handler;
}
