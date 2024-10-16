///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  The Programmable Interval Timer driver.
///
/// Contains functions for programming the PIT.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <pit.h>
#include <pic.h>
#include <irq.h>
#include <tty.h>
#include <isr.h>
#include <x86.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the timer.
///
/// @param  freq    The frequency the timer should operate on.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pit_init(u64 freq) {

    tty_puts(WHITE_ON_BLACK, "Setting up PIT...");

    irq_add(0, pit_handler);

    u64 divisor = PIT_FREQUENCY / freq;

    x86_outb(PIT_CMD, CMD_BIN | CMD_MODE2 | CMD_RW_LOW_HI | CMD_CHANNEL0);

    // send low and high bytes of divisor
    x86_outb(PIT_DATA_0, (u8)divisor & 0xFF);
    x86_outb(PIT_DATA_0, (u8)((divisor >> 8) & 0xFF));

    pic_unmask_irq(0);
    tty_puts(WHITE_ON_BLACK, "Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  PIT handler.
///
/// @param  args    The Trapframe passed on by the original ISR.
///
/// Executes every timer interval.
///////////////////////////////////////////////////////////////////////////////////////////////////

void pit_handler(int_args_t *args) {

    static u64 counter;
    counter++;
    u64 off = tty_get_cursor();
    tty_set_cursor(0);
    tty_putu(WHITE_ON_BLACK, counter);
    tty_set_cursor(off);
}
