///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains functions for handling exceptions/errors.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <tty.h>
#include <idt.h>
#include <isr.h>
#include <err.h>
#include <tty.h>
#include <x86.h>
#include <dbg.h>

    
/// @brief  Array containing exception messages for CPU reserved interrupts.
const char *msg[] = {
        "Division By Zero",
        "Debug",
        "Non Maskable Interrupt",
        "Breakpoint",
        "Into Detected Overflow",
        "Out of Bounds",
        "Invalid Opcode",
        "No Coprocessor",

        "Double Fault",
        "Coprocessor Segment Overrun",
        "Bad TSS",
        "Segment Not Present",
        "Stack Fault",
        "General Protection Fault",
        "Page Fault",
        "Unknown Interrupt",

        "Coprocessor Fault",
        "Alignment Check",
        "Machine Check",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",

        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved",
        "Reserved"
};


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Invoked when an exception occures.
///
/// @param  args    A pointer to the current trapframe.
///
/// @warning    Does not return.
///////////////////////////////////////////////////////////////////////////////////////////////////

NORETURN void err_handler(int_args_t *args) {

    dbg_warn(
            "Exception occured!\n \
            rax=%x\n \
            rbx=%x\n \
            rcx=%x\n \
            rdx=%x\n \
            rdi=%x\n \
            rsi=%x\n \
            r8=%x\n \
            r9=%x\n \
            r10=%x\n \
            r11=%x\n \
            r12=%x\n \
            r13=%x\n \
            r14=%x\n \
            r15=%x\n \
            cr0=%x\n \
            cr2=%x\n \
            cr3=%x\n \
            cr4=%x\n \
            int_num=%u\n \
            err_code=%x\n \
            rip=%x\n \
            cs=%x\n \
            flags=%x\n \
            rsp=%x\n \
            ds=%x\n \
            ", 
            args->general_regs.rax,
            args->general_regs.rbx,
            args->general_regs.rcx,
            args->general_regs.rdx,
            args->general_regs.rdi,
            args->general_regs.rsi,
            args->general_regs.r8,
            args->general_regs.r9,
            args->general_regs.r10,
            args->general_regs.r11,
            args->general_regs.r12,
            args->general_regs.r13,
            args->general_regs.r14,
            args->general_regs.r15,
            args->control_regs.cr0,
            args->control_regs.cr2,
            args->control_regs.cr3,
            args->control_regs.cr4,
            args->int_vec,
            args->err_code,
            args->rip,
            args->cs,
            args->flags,
            args->rsp,
            args->ds
            );

    tty_putf(MIX(BLACK, RED), "\nException occured: %s!\n", msg[args->int_vec]);
    x86_hang();
}
