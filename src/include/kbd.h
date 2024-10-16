///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  Keyboard driver header file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>
#include <isr.h>


#define IRQ_KEYBOARD        1
#define KBD_PORT            0x60


#define KEY_SHIFT           42
#define KEY_SHIFT_RELEASE   170
#define KEY_DEL             14
#define KEY_ARROW_UP        72
#define KEY_ARROW_DOWN      80
#define KEY_ARROW_LEFT      75
#define KEY_ARROW_RIGHT     77


void kbd_init(void);
void kbd_handler(int_args_t *args);
