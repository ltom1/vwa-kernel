///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Debug driver header file.
///
/// Contains declarations and type definitions used when printing debug messages.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define DBG_PORT 0xE9


void dbg_info(const char *fmt, ...);
void dbg_warn(const char *fmt, ...);
void NORETURN dbg_err(const char *fmt, ...);

