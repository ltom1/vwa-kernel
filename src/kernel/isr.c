///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for handling/registering Interrupt Service Routines.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <isr.h>
#include <idt.h>
#include <irq.h>
#include <err.h>
#include <x86.h>
#include <tty.h>
#include <proc.h>
#include <syscalls.h>


/// @brief  Table of exception/IRQ isr stub function pointers (in assembly).
extern u64 isr_stub_table[MAX_ERR + MAX_IRQ + 1];
/// @brief  Syscall isr stub function pointer (in assembly).
extern u64 isr_syscall;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes Interrupt Service Routines.
///////////////////////////////////////////////////////////////////////////////////////////////////

void isr_init(void) {

    // exceptions:  0       - MAX_ERR
    // IRQs:        MAX_ERR - MAX_IRQ
    for (u64 i = 0; i < MAX_ERR + MAX_IRQ; i++) {
        idt_set_desc(i, isr_stub_table[i], IDT_INT_GATE | IDT_PRESENT, IST0);
    }
    // syscalls
    idt_set_desc(SYSCALL_VEC, isr_syscall, IDT_SYSCALL | IDT_USER_ACCESS | IDT_PRESENT, IST0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Handles Interrupts. Called by isr_common.
///
/// @param  args    A pointer to the current trapframe.
///////////////////////////////////////////////////////////////////////////////////////////////////

void isr_handler(int_args_t *args) {
    
    // exception
    if (args->int_vec < MAX_ERR) {
        err_handler(args);
        // should not return
    }
    // IRQ
    if (args->int_vec < MAX_ERR + MAX_IRQ) {
        irq_handler(args);
    } else 
    // syscall
    if (args->int_vec == SYSCALL_VEC) {
       syscall_handler(args);
    }
}
