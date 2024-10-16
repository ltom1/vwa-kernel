///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  PIT driver header file.
///
/// Contains declarations and type definitions. 
/// Used when programming the Progammable Interval Timer.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <isr.h>
#include <types.h>

#define PIT_FREQUENCY           1193182

// IO Ports
#define PIT_DATA_0              0x40
#define PIT_DATA_1              0x41
#define PIT_DATA_2              0x42
#define PIT_CMD                 0x43


// Commands

// BCD/Binary mode
#define CMD_BIN                 0x00
#define CMD_BCD                 0x01

// Select Channel
#define CMD_CHANNEL0            0x00
#define CMD_CHANNEL1            0x40
#define CMD_CHANNEL2            0x80
#define CMD_READBACK            0xc0

// Access Mode
#define CMD_LATCH               0x00
#define CMD_RW_LOW              0x10
#define CMD_RW_HI               0x20
#define CMD_RW_LOW_HI           0x30

// Operating Mode
#define CMD_MODE0               0x00    // Interrupt on Terminal Count
#define CMD_MODE1               0x02    // Hardware Retriggerable One-Shot
#define CMD_MODE2               0x04    // Rate Generator
#define CMD_MODE3               0x06    // Square Wave
#define CMD_MODE4               0x08    // Software Trigerred Strobe
#define CMD_MODE5               0x0a    // Hardware Trigerred Strobe


void pit_init(u64 freq);
void pit_handler(int_args_t *args);
