///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file 
/// @brief  Debug driver.
/// 
/// Contains code for printing debug messages.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <dbg.h>
#include <x86.h>

#include <stdarg.h>


void dbg_putf(const char *fmt, va_list vl);

void dbg_putc(char c);
void dbg_putd(s64 num);
void dbg_putu(u64 num);
void dbg_putx(u64 hex);
void dbg_puts(const char *str);


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Writes information to debug.
///
/// @param  fmt     The format string.
/// @param  ...     The values to print into the string.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_info(const char *fmt, ...) {

    va_list vl;
    va_start(vl, fmt);

    dbg_puts("\033[32m[INFO]   : ");
    dbg_putf(fmt, vl);

    va_end(vl);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Writes a warning to debug.
///
/// @param  fmt     The format string.
/// @param  ...     The values to print into the string.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_warn(const char *fmt, ...) {

    va_list vl;
    va_start(vl, fmt);

    dbg_puts("\033[33m[WARNING]: ");
    dbg_putf(fmt, vl);

    va_end(vl);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Writes an error message to debug and hangs the CPU.
///
/// @warning    Does not return.
///
/// @param  fmt     The format string.
/// @param  ...     The values to print into the string.
///////////////////////////////////////////////////////////////////////////////////////////////////

void NORETURN dbg_err(const char *fmt, ...) {

    va_list vl;
    va_start(vl, fmt);

    dbg_puts("\033[31m[ERROR]  : ");
    dbg_putf(fmt, vl);

    va_end(vl);

    x86_hang();
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Writes char to debug.
///
/// @param  c   The character.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_putc(char c) {
    
    x86_outb(DBG_PORT, c);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a formatted string to debug.
///
/// @param  fmt     The format string.
/// @param  vl      The argument va_list.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_putf(const char *fmt, va_list vl) {

    char c;
    for (u64 i = 0; (c = fmt[i]) != 0; i++) {
        
        if (c != '%') {
            dbg_putc(c);
            continue;
        }

        i++;
        c = fmt[i];
        switch (c) {
            case 'd':
                dbg_putd((u64)va_arg(vl, u64));
                break;
            case 'u':
                dbg_putu((s64)va_arg(vl, u64));
                break;
            case 's':
                dbg_puts((const char*)va_arg(vl, u64));
                break;
            case 'c':
                dbg_putc((char)va_arg(vl, u64));
                break;
            case 'x':
            case 'p':
                dbg_putx((u64)va_arg(vl, u64));
                break;
            case '%':
                dbg_putc('%');
                break;
            default:
                dbg_putc('%');
                dbg_putc(c);
                break;
        }
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints an unsigned integer (u64) as a decimal to debug.
///
/// @param  num     The unsigned integer.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_putu(u64 num) {

    char buf[16];
    s64 i = 0;

    do {
        buf[i++] = num % 10 + '0';
    } while ((num /= 10) != 0);

    while (--i >= 0)
        dbg_putc(buf[i]);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a signed integer (s64) as a decimal to debug.
///
/// @param  num     The signed integer.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_putd(s64 num) {

    if (num < 0) {
        dbg_putc('-');
        num *= -1;
    }
    dbg_putu(num);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints an unsigned integer (u64) as a hexadecimal to debug.
///
/// @param  hex     The unsigned integer.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_putx(u64 hex) {

    u32 tmp;
    for (s32 i = 15; i >= 0; i--) {
        tmp = hex;
        tmp = tmp >> i * 4;
        tmp &= 0xf;
        tmp += ((tmp < 10) ? '0' : 'a' - 10);
        dbg_putc(tmp);
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Prints a null-terminated string to debug.
///
/// @param  str     The null-terminated string.
///////////////////////////////////////////////////////////////////////////////////////////////////

void dbg_puts(const char *str) {

    u32 i = 0;
    char c = str[i];
    while (c != 0) {
        dbg_putc(c);
        i++;
        c = str[i];
    }
}
