///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file 
/// @brief  TTY driver.
/// 
/// Contains code for printing to the screen.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <tty.h>
#include <types.h>
#include <x86.h>
#include <utils.h>

#include <stdarg.h>


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Clears the screen.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_clear_screen(void) {

    u8 *vga = (u8*)VGA_ADDR;

    for (u64 i = 0; i < MAX_COLS * MAX_ROWS; i++) {
        vga[2 * i] = ' ';
        vga[2 * i + 1] = WHITE_ON_BLACK;
    }
    tty_set_cursor(0);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the tty display.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_init(void) {

    tty_clear_screen();
    tty_enable_cursor();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Enables the cursor.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_enable_cursor(void) {

    x86_outb(VGA_CTRL_REG, VGA_CURSOR_CMD);
    x86_outb(VGA_DATA_REG, VGA_CURSOR_ON);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Disables the cursor.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_disable_cursor(void) {

    x86_outb(VGA_CTRL_REG, VGA_CURSOR_CMD);
    x86_outb(VGA_DATA_REG, VGA_CURSOR_OFF);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Moves the cursor to the specified offset.
///
/// @param  off     The character offset from the start of the screen.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_set_cursor(u64 off) {

    x86_outb(VGA_CTRL_REG, VGA_CURSOR_OFF_HIGH);
    x86_outb(VGA_DATA_REG, HIGH_BYTE(off));

    x86_outb(VGA_CTRL_REG, VGA_CURSOR_OFF_LOW);
    x86_outb(VGA_DATA_REG, LOW_BYTE(off));
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Gets the cursor's character offset from the start of the screen.
///
/// @returns    The character offset from the start of the screen.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 tty_get_cursor(void) {

    // programs the data register to show the high byte of the offset 
    // and loads it into the high byte of offset (<< shifting 8bits=1byte)
    x86_outb(VGA_CTRL_REG, VGA_CURSOR_OFF_LOW);
    u64 off = x86_inb(VGA_DATA_REG);

    // programs the data register to show the low byte of the offset 
    // and loads it into the low byte of offset (adding)
    x86_outb(VGA_CTRL_REG, VGA_CURSOR_OFF_HIGH);
    off += x86_inb(VGA_DATA_REG) << 8;

    return off;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Converts a two-dimensional screen position to an one-dimensional offset.
///
/// @param  col     The column or x value of the position.
/// @param  row     The row or y value of the position.
///
/// @returns    The character offset from the start of the screen.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 tty_pos_to_off(u64 col, u64 row) {

    return row * MAX_COLS + col;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Calculates the character offset to the next new line after a given offset.
///
/// @param  off     The current offset.
///
/// @returns    The character offset from the start of the screen of the next line after off.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 tty_offset_new_line(u64 off) {

    // get current row
    u64 row = off / MAX_COLS;

    // return offset of 1st (0) column of the next row
    return tty_pos_to_off(0, row + 1);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Checks if text reached the end and scrolls the screen if it has.
///
/// @param  off     The current offset (will be checked).
///
/// @returns    The new offset (new line if the screen had to be scrolled).
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 tty_check_scroll(u64 off) {

    // check if scrolling is needed
    if (off >= MAX_ROWS * MAX_COLS) return tty_scroll_down();

    // else keep offset
    return off;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Scrolls one line down.
///
/// @returns    The start offset of the new line created by scrolling.
///////////////////////////////////////////////////////////////////////////////////////////////////

u64 tty_scroll_down(void) {

    u8 *vga = (u8*)VGA_ADDR;

    // copy second to last row to first row
    mem_cpy(
        vga,
        vga + MAX_COLS * 2,
        MAX_COLS * (MAX_ROWS - 1) * 2
        );

    // clear last row
    u64 off_last_row = tty_pos_to_off(0, MAX_ROWS - 1) * 2;
    for (u64 col = 0; col < MAX_COLS; col++) {
        vga[off_last_row + col * 2] = ' ';
        vga[off_last_row + col * 2 + 1] = WHITE_ON_BLACK;
    }

    off_last_row /= 2;
    tty_set_cursor(off_last_row);
    return off_last_row;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a character at the cursor position.
///
/// @param  attr    The character's color attributes.
/// @param  c       The character to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_putc(u8 attr, char c) {

     u64 off = tty_get_cursor();
     u8 *vga = (u8*)VGA_ADDR;

     if (!attr) {
          attr = WHITE_ON_BLACK;
     }

     if (c == '\n') {
          off = tty_offset_new_line(off) - 1;
     } else {
          vga[2 * off] = c;
          vga[2 * off + 1] = attr;
     }

     off++;
     off = tty_check_scroll(off);
     tty_set_cursor(off);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a formatted string to the cursor position.
///
/// @param  attr    The strings's color attributes.
/// @param  c       The format string.
/// @param  ...     The values to put in.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_putf(u8 attr, const char *fmt, ...) {

    va_list vl;
    va_start(vl, fmt);

    char c;
    for (u64 i = 0; (c = fmt[i]) != 0; i++) {
        
        if (c != '%') {
            tty_putc(attr, c);
            continue;
        }

        i++;
        c = fmt[i];
        switch (c) {
            case 'd':
                tty_putd(attr, (u64)va_arg(vl, u64));
                break;
            case 'u':
                tty_putu(attr, (s64)va_arg(vl, u64));
                break;
            case 's':
                tty_puts(attr, (const char*)va_arg(vl, u64));
                break;
            case 'c':
                tty_putc(attr, (char)va_arg(vl, u64));
                break;
            case 'x':
            case 'p':
                tty_putx(attr, (u64)va_arg(vl, u64));
                break;
            case '%':
                tty_putc(attr, '%');
                break;
            default:
                tty_putc(attr, '%');
                tty_putc(attr, c);
                break;
        }
    }

    va_end(vl);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints an unsigned integer (u64) at the cursor position in decimal.
///
/// @param  attr    The unsigned integer's color attributes.
/// @param  num     The unsigned integer to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_putu(u8 attr, u64 num) {

    char buf[16];
    s64 i = 0;

    do {
        buf[i++] = num % 10 + '0';
    } while ((num /= 10) != 0);

    while (--i >= 0)
        tty_putc(attr, buf[i]);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a signed integer (s64) at the cursor position in decimal.
///
/// @param  attr    The signed integer's color attributes.
/// @param  num     The signed integer to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_putd(u8 attr, s64 num) {

    if (num < 0) {
        tty_putc(attr, '-');
        num *= -1;
    }
    tty_putu(attr, num);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints an unsigned integer (u64) at the cursor position in hexadecimal.
///
/// @param  attr    The unsigned integer's color attributes.
/// @param  hex     The unsigned integer to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_putx(u8 attr, u64 hex) {

    u64 tmp;
    for (s64 i = 15; i >= 0; i--) {
        tmp = hex;
        tmp = tmp >> i * 4;
        tmp &= 0xf;
        tmp += ((tmp < 10) ? '0' : 'a' - 10);
        tty_putc(attr, tmp);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a string to the cursor position.
///
/// @param  attr    The string's color attributes.
/// @param  str     The string to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void tty_puts(u8 attr, const char *str) {

    u64 i = 0;
    char c = str[i];
    while (c != 0) {
        tty_putc(attr, c);
        i++;
        c = str[i];
    }
}
