///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  The Keyboard driver.
///
/// Contains functions for programming the Keyboard.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <x86.h>
#include <tty.h>
#include <kbd.h>
#include <irq.h>
#include <pic.h>
#include <isr.h>


/// @brief  Scancode to character mapping for lowercase characters.
char low_keymap[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'S', '`', 0,
    '\t', 'q', 'w', 'e', 'r', 't', 'z', 'u', 'i', 'o', 'p', 'u', '+', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'o', 'a', '^', 0, '#',
    'y', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '-',
    0,'*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, 
    '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/// @brief  Scancode to character mapping for uppercase characters.
char high_keymap[] = {
    0, 0, '!', '"', '3', '$', '%', '&', '/', '(', ')', '=', '?', '`', 0,
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Z', 'U', 'I', 'O', 'P', 'U', '*', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'O', 'A', '^', 0, '\'',
    'Y', 'X', 'C', 'V', 'B', 'N', 'M', ';', ':', '_',
    0,'*', 0, ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, 
    '+', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

/// @brief  Pointer to the current keymap.
char *current_keymap = low_keymap;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the Keyboard.
///////////////////////////////////////////////////////////////////////////////////////////////////

void kbd_init(void) {

    tty_puts(WHITE_ON_BLACK, "Setting up Keyboard...");

    irq_add(IRQ_KEYBOARD, kbd_handler);
    pic_unmask_irq(IRQ_KEYBOARD);

    // todo: maybe read a few scancodes from KEYBOARD_PORT 
    // (there might still be scan codes blocking the keyboard interrupts)

    tty_puts(WHITE_ON_BLACK, "Done!\n");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Keyboard handler.
///
/// @param  args    The Trapframe passed on by the original ISR.
///
/// Prints pressed keys to the screen and allows cursur movement via the arrow keys.
/// Also supports uppercase/lowercase.
///////////////////////////////////////////////////////////////////////////////////////////////////

void kbd_handler(int_args_t *args) {

    u8 scancode = x86_inb(KBD_PORT);

    if (scancode == KEY_SHIFT) {
        current_keymap = high_keymap;
        return;
    } else if (scancode == KEY_SHIFT_RELEASE) {
        current_keymap = low_keymap;
        return;
    }

    // ignore key releases
    if ((scancode & 0x80) != 0) return;

    if (scancode == KEY_DEL) {
        tty_set_cursor(tty_get_cursor() - 1);
        tty_putc(WHITE_ON_BLACK, ' ');
        tty_set_cursor(tty_get_cursor() - 1);
        return;
    }

    // move with arrow keys
    switch (scancode) {
        case KEY_ARROW_UP:
            tty_set_cursor(tty_get_cursor() - MAX_COLS);
            return;
        case KEY_ARROW_DOWN:
            tty_set_cursor(tty_get_cursor() + MAX_COLS);
            return;
        case KEY_ARROW_LEFT:
            tty_set_cursor(tty_get_cursor() - 1);
            return;
        case KEY_ARROW_RIGHT:
            tty_set_cursor(tty_get_cursor() + 1);
            return;
    }

    tty_putc(WHITE_ON_BLACK, current_keymap[scancode]);
}
