///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Contains function declarations for handling errors and exceptions.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <isr.h>


#define MAX_ERR    32

#define panic(fmt, ...) { \
    tty_puts(MIX(BLACK, RED), "\nPanic: "); \
    tty_putf(MIX(BLACK, RED), fmt __VA_OPT__(,) __VA_ARGS__); \
    tty_disable_cursor(); \
    x86_hang(); }


extern void err_handler(int_args_t *args);
