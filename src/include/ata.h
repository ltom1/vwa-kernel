///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  ATA driver header file.
///////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once


#include <types.h>


#define MODE_IDENTIFY               1
#define MODE_LBA28                  2
#define MODE_LBA48                  3

#define TYPE_NONE                   0
#define TYPE_ATA                    1
#define TYPE_ATAPI                  2
#define TYPE_SATA                   3

#define ATAPI_LBA1                  0x14
#define ATAPI_LBA2                  0xEB

#define SATA_LBA1                   0x3c
#define SATA_LBA2                   0xc3

#define ATA_PRIMARY                 0x1F0
#define ATA_SECONDARY               0x170

#define ATA_OFF_DATA                0x00
#define ATA_OFF_ERROR               0x01
#define ATA_OFF_FEATURES            0x01
#define ATA_OFF_SECCOUNT0           0x02
#define ATA_OFF_LBA0                0x03
#define ATA_OFF_LBA1                0x04
#define ATA_OFF_LBA2                0x05
#define ATA_OFF_DRIVE_SELECT        0x06
#define ATA_OFF_CMD                 0x07
#define ATA_OFF_STATUS              0x07
#define ATA_OFF_SECCOUNT1           0x08
#define ATA_OFF_LBA3                0x09
#define ATA_OFF_LBA4                0x0A
#define ATA_OFF_LBA5                0x0B
#define ATA_OFF_CTRL                0x0C
#define ATA_OFF_ALTSTATUS           0x0C
#define ATA_OFF_DEVADDRESS          0x0D
#define ATA_OFF_ALTCTRL             0x206

#define ATA_IDENTIFY_SELECT_MASTER  0xA0
#define ATA_IDENTIFY_SELECT_SLAVE   0xB0

#define ATA_LBA28_SELECT_MASTER     0xE0
#define ATA_LBA28_SELECT_SLAVE      0xF0

#define ATA_LBA48_SELECT_MASTER     0x40
#define ATA_LBA48_SELECT_SLAVE      0x50

#define ATA_CMD_IDENTIFY            0xEC
#define ATA_CMD_READ28              0x20
#define ATA_CMD_WRITE28             0x30
#define ATA_CMD_READ48              0x24
#define ATA_CMD_WRITE48             0x34

#define ATA_CMD_FLUSH               0xE7


/// @brief  Structure describing an ATA drive.
typedef struct ATA {
    bool primary;
    bool master;
    u8 type;
    u32 secs28;
    u64 secs48;
} ata_t;


extern ata_t boot_drive;

void ata_init(void);

void ata_select_drive(ata_t drive, u8 mode, u32 lba);
ata_t ata_identify(ata_t drive);

void ata_read28(ata_t drive, u8 *dest, u32 lba, u8 num_secs);
void ata_write28(ata_t drive, u8 *src, u32 lba, u8 num_secs);

void ata_read48(ata_t drive, u8 *dest, u64 lba, u16 num_secs);
void ata_write48(ata_t drive, u8 *src, u64 lba, u16 num_secs);

void ata_400ns_delay(ata_t drive);
void ata_flush(ata_t drive, u8 mode);
