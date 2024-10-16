///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  TTY driver header file.
///
/// Contains declarations and type definitions used when printing to the screen.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>

#include <stdarg.h>


#define VGA_ADDR         0xc00b8000

// text mode dimensions
#define MAX_ROWS         25
#define MAX_COLS         80

// defaults
#define WHITE_ON_BLACK   0x0f

// individual colors
#define   BLACK          0x00
#define   BLUE           0x01
#define   GREEN          0x02
#define   CYAN           0x03
#define   RED            0x04
#define   MAGENTA        0x05
#define   BROWN          0x06
#define   LIGHT_GRAY     0x07
#define   DARK_GRAY      0x08
#define   LIGHT_BLUE     0x09
#define   LIGHT_GREEN    0x0a
#define   LIGHT_CYAN     0x0b
#define   LIGHT_RED      0x0c
#define   LIGHT_MAGENTA  0x0d
#define   YELLOW         0x0e
#define   WHITE          0x0f

// macro to create color compositions
#define MIX(fg, bg) ((bg << 4) ^ fg)

// vga control ports
#define VGA_CTRL_REG            0x3d4
#define VGA_DATA_REG            0x3d5

// have to be in VGA_CTRL_REG for VGA_DATA_REG to hold low or high byte of the offset
#define VGA_CURSOR_OFF_HIGH     0x0e
#define VGA_CURSOR_OFF_LOW      0x0f

// values to enable/disable the cursor
#define VGA_CURSOR_CMD          0x0a
#define VGA_CURSOR_ON           0x00
#define VGA_CURSOR_OFF          0x20


/// @brief  Struct of a VGA character
typedef struct PACKED VGAChar {
    u8 c;
    u8 attr;
} vga_char_t;

void tty_clear_screen(void);
void tty_init(void);
void tty_enable_cursor(void);
void tty_disable_cursor(void);
void tty_set_cursor(u64 off);
u64 tty_get_cursor(void);
u64 tty_pos_to_off(u64 col, u64 row);
u64 tty_offset_new_line(u64 off);
u64 tty_check_scroll(u64 off);
u64 tty_scroll_down(void);

void tty_putc(u8 attr, char c);
void tty_putf(u8 attr, const char *fmt, ...);
void tty_putu(u8 attr, u64 num);
void tty_putd(u8 attr, s64 num);
void tty_putx(u8 attr, u64 hex);
void tty_puts(u8 attr, const char *str);
