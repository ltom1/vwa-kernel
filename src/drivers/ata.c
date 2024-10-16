///////////////////////////////////////////////////////////////////////////////////////////////////
/// @file
/// @brief  The Keyboard driver.
///
/// Contains functions for programming the Keyboard.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include <types.h>
#include <x86.h>
#include <err.h>
#include <tty.h>
#include <ata.h>


bool sel_primary = -1;
bool sel_master = -1;
u8 sel_mode = -1;
u32 sel_lba = -1;

ata_t boot_drive;


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Initializes the main ATA drive.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_init(void) {

    bool master = (x86_inb(ATA_PRIMARY + ATA_OFF_DRIVE_SELECT) == ATA_LBA28_SELECT_MASTER);
    boot_drive = ata_identify((ata_t) { 
            true, 
            master,
            TYPE_NONE,
            0,
            0
            });
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Reads sectors from an ATA drive into RAM using LBA48.
///
/// @param  drive       The drive to read from.
/// @param  dest        Where to put the data.
/// @param  lba         The number of the first sector to read.
/// @param  num_secs    How many sectors to read.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_read48(ata_t drive, u8 *dest, u64 lba, u16 num_secs) {

    u8 secs_read = 0;

    if (lba & 0xFFFF000000000000) panic("LBA is larger than 48 bits");

    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    ata_select_drive(drive, MODE_LBA48, 0);

    // clear error port
    x86_outb(port_base + ATA_OFF_ERROR, 0);

    // send sector count high byte
    x86_outb(port_base + ATA_OFF_SECCOUNT0, (num_secs >> 8) & 0xFF);

    // send start lba second half
    x86_outb(port_base + ATA_OFF_LBA0, (lba >> 24) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA1, (lba >> 32) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA2, (lba >> 40) & 0xFF);

    // send sector count low byte
    x86_outb(port_base + ATA_OFF_SECCOUNT0, num_secs & 0xFF);

    // send start lba first half
    x86_outb(port_base + ATA_OFF_LBA0, lba & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA1, (lba >> 8) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA2, (lba >> 16) & 0xFF);

    // send READ 48 command
    x86_outb(port_base + ATA_OFF_CMD, ATA_CMD_READ48);

    while (secs_read < num_secs) {
    
        // wait for drive
        ata_400ns_delay(drive);

        // poll
        u8 status = x86_inb(port_base + ATA_OFF_STATUS);
        while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) 
            status = x86_inb(port_base + ATA_OFF_STATUS);

        if (status & 0x01) panic("Error reading disk");

        for (u64 i = 0; i < 256; i++) {
            *(u16*)(dest + secs_read * 512 + i * 2) = x86_inw(port_base + ATA_OFF_DATA);
        }
        secs_read++;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Writes the specified sectors from RAM onto an ATA drive using LBA48.
///
/// @param  drive       The drive to write to.
/// @param  src         The address of the data to write.
/// @param  lba         The LBA address to write to.
/// @param  num_secs    The amount of sectors to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_write48(ata_t drive, u8 *src, u64 lba, u16 num_secs) {

    u8 secs_written = 0;

    if (lba & 0xFFFF000000000000) panic("LBA is larger than 48 bits");

    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    ata_select_drive(drive, MODE_LBA48, 0);

    // clear error port
    x86_outb(port_base + ATA_OFF_ERROR, 0);

    // send sector count high byte
    x86_outb(port_base + ATA_OFF_SECCOUNT0, (num_secs >> 8) & 0xFF);

    // send start lba second half
    x86_outb(port_base + ATA_OFF_LBA0, (lba >> 24) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA1, (lba >> 32) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA2, (lba >> 40) & 0xFF);

    // send sector count low byte
    x86_outb(port_base + ATA_OFF_SECCOUNT0, num_secs & 0xFF);

    // send start lba first half
    x86_outb(port_base + ATA_OFF_LBA0, lba & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA1, (lba >> 8) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA2, (lba >> 16) & 0xFF);

    // send WRITE 48 command
    x86_outb(port_base + ATA_OFF_CMD, ATA_CMD_WRITE48);

    u16 data;
    while (secs_written < num_secs) {

        for (u64 i = 0; i < 256; i++) {
            data = *(u16*)(src + secs_written * 512 + i * 2);
            x86_outw(port_base + ATA_OFF_DATA, data);
        }

        ata_flush(drive, MODE_LBA28);
        secs_written++;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Reads sectors from an ATA drive into RAM using LBA28.
///
/// @param  drive       The drive to read from.
/// @param  dest        Where to put the data.
/// @param  lba         The number of the first sector to read.
/// @param  num_secs    How many sectors to read.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_read28(ata_t drive, u8 *dest, u32 lba, u8 num_secs) {

    u8 secs_read = 0;

    if (lba & 0xF0000000) panic("LBA is larger than 28 bits");

    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    ata_select_drive(drive, MODE_LBA28, lba);

    // clear error port
    x86_outb(port_base + ATA_OFF_ERROR, 0);

    // send sector count
    x86_outb(port_base + ATA_OFF_SECCOUNT0, num_secs);

    // send start lba
    x86_outb(port_base + ATA_OFF_LBA0, lba & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA1, (lba >> 8) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA2, (lba >> 16) & 0xFF);

    // send READ 28 command
    x86_outb(port_base + ATA_OFF_CMD, ATA_CMD_READ28);

    while (secs_read < num_secs) {
    
        // wait for drive
        ata_400ns_delay(drive);

        // poll
        u8 status = x86_inb(port_base + ATA_OFF_STATUS);
        while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) 
            status = x86_inb(port_base + ATA_OFF_STATUS);

        if (status & 0x01) panic("Error reading disk");

        for (u64 i = 0; i < 256; i++) {
            *(u16*)(dest + secs_read * 512 + i * 2) = x86_inw(port_base + ATA_OFF_DATA);
        }
        secs_read++;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Writes the specified sectors from RAM onto an ATA drive using LBA28.
///
/// @param  drive       The drive to write to.
/// @param  src         The address of the data to write.
/// @param  lba         The LBA address to write to.
/// @param  num_secs    The amount of sectors to write.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_write28(ata_t drive, u8 *src, u32 lba, u8 num_secs) {

    u8 secs_written = 0;

    if (lba & 0xF0000000) panic("LBA is larger than 28 bits");

    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    ata_select_drive(drive, MODE_LBA28, lba);

    // clear error port
    x86_outb(port_base + ATA_OFF_ERROR, 0);

    // send sector count
    x86_outb(port_base + ATA_OFF_SECCOUNT0, num_secs);

    // send start lba
    x86_outb(port_base + ATA_OFF_LBA0, lba & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA1, (lba >> 8) & 0xFF);
    x86_outb(port_base + ATA_OFF_LBA2, (lba >> 16) & 0xFF);

    // send WRITE 28 command
    x86_outb(port_base + ATA_OFF_CMD, ATA_CMD_WRITE28);

    u16 data;
    while (secs_written < num_secs) {

        for (u64 i = 0; i < 256; i++) {
            data = *(u16*)(src + secs_written * 512 + i * 2);
            x86_outw(port_base + ATA_OFF_DATA, data);
        }

        ata_flush(drive, MODE_LBA28);
        secs_written++;
    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Flushes write cache of an ATA drive to avoid bad/corrupted sectors after writes.
///
/// @param  drive   The drive to flush.
/// @param  mode    The write mode (LBA28/LBA48).
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_flush(ata_t drive, u8 mode) {

    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    ata_select_drive(drive, mode, 0);

    // send FLUSH command
    x86_outb(port_base + ATA_OFF_CMD, ATA_CMD_FLUSH);
    
    // poll
    u8 status = x86_inb(port_base + ATA_OFF_STATUS);
    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) 
        status = x86_inb(port_base + ATA_OFF_STATUS);

    if (status & 0x01) panic("Error flushing disk cache");
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Waits for 400ns for an ATA drive. Used when selecting a drive.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_400ns_delay(ata_t drive) {

    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    for (u64 i = 0; i < 4; i++)
        x86_inb(port_base + ATA_OFF_ALTSTATUS);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Tries to identify an IDE ATA device and determines its capacity.
///
/// @param  drive   Information of the drive to identify.
///
/// @returns    The drive with updated information.
///////////////////////////////////////////////////////////////////////////////////////////////////

ata_t ata_identify(ata_t drive) {

    ata_t res = { drive.primary, drive.master };
    u16 port_base = drive.primary ? ATA_PRIMARY : ATA_SECONDARY;

    ata_select_drive(drive, MODE_IDENTIFY, 0);

    // clear status port
    x86_outb(port_base + ATA_OFF_ALTCTRL, 0);

    // check if there are drives connected to the bus
    if (x86_inb(port_base + ATA_OFF_STATUS) == 0xFF) return res;
    
    ata_select_drive(drive, MODE_IDENTIFY, 0);

    // clear registers
    x86_outb(port_base + ATA_OFF_SECCOUNT0, 0);
    x86_outb(port_base + ATA_OFF_LBA0, 0);
    x86_outb(port_base + ATA_OFF_LBA1, 0);
    x86_outb(port_base + ATA_OFF_LBA2, 0);

    // send IDENTIFY command
    x86_outb(port_base + ATA_OFF_CMD, ATA_CMD_IDENTIFY);

    // check if drive is connected
    u8 status = x86_inb(port_base + ATA_OFF_STATUS);
    if (status == 0x00) return res;

    while (((status & 0x80) == 0x80) && ((status & 0x01) != 0x01)) 
        status = x86_inb(port_base + ATA_OFF_STATUS);

    if (status & 0x01) {

        u8 lba1 = x86_inb(port_base + ATA_OFF_LBA1);
        u8 lba2 = x86_inb(port_base + ATA_OFF_LBA2);

        if ((lba1 == ATAPI_LBA1) && (lba2 == ATAPI_LBA2)) {
            res.type = TYPE_ATAPI;
            return res;
        }

        if ((lba1 == SATA_LBA1) && (lba2 == SATA_LBA2)) {
            res.type = TYPE_SATA;
            return res;
        }

        return res;
    }
    
    // look through drive information
    u16 cur_data;
    for (u64 i = 0; i < 256; i++) {
        
    
        cur_data = x86_inw(port_base + ATA_OFF_DATA);

        // check if LBA28 is supported
        if (i == 60) {

            // only 1 read -> no temporary variables needed
            drive.secs28 = DWORD(
                    x86_inw(port_base + ATA_OFF_DATA),
                    cur_data    // lower half was the last read
                    );
            i++;
        }

        // check if LBA48 is supported
        else if (i == 100) {

            // we need some temporary variables here to preserve the correct word order
            // word0 = cur_data
            u16 word1 = x86_inw(port_base + ATA_OFF_DATA);
            u16 word2 = x86_inw(port_base + ATA_OFF_DATA);
            u16 word3 = x86_inw(port_base + ATA_OFF_DATA);

            drive.secs48 = QWORD(
                DWORD(
                    word3, 
                    word2), 
                DWORD(
                    word1,
                    cur_data)
                );
            i += 3;
        }
    }

    res.type = TYPE_ATA;
    return res;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/// @brief  Selects/programs an ATA drive.
///
/// @param  drive   The drive to select.
/// @param  mode    The mode (e.g. LBA28, LBA48, IDENTIFY).
/// @param  lba     The LBA address to select.
///////////////////////////////////////////////////////////////////////////////////////////////////

void ata_select_drive(ata_t drive, u8 mode, u32 lba) {

    if ((drive.primary == sel_primary) && 
            (drive.master == sel_master) &&
            (mode == sel_mode) &&
            (lba == sel_lba))
        return;

    u8 data;
    switch (mode) {
        case MODE_IDENTIFY:
            data = drive.master ? ATA_IDENTIFY_SELECT_MASTER : ATA_IDENTIFY_SELECT_SLAVE;
            break;
        case MODE_LBA28:
            data = drive.master ? ATA_LBA28_SELECT_MASTER : ATA_LBA28_SELECT_SLAVE;
            data |= (u8)(lba >> 24) & 0x0F;
            break;
        case MODE_LBA48:
            data = drive.master ? ATA_LBA48_SELECT_MASTER : ATA_LBA48_SELECT_SLAVE;
            break;
        default:
            panic("No selection mode selected");
    }

    x86_outb(
            drive.primary ? ATA_PRIMARY + ATA_OFF_DRIVE_SELECT : ATA_SECONDARY + ATA_OFF_DRIVE_SELECT,
            data
            );

    sel_lba = lba;
    sel_mode = mode;
    sel_primary = drive.primary;
    sel_master = drive.master;
}
