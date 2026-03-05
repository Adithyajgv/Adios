#include "x86_64/ata.h"
#include "x86_64/port.h"
#include "bool.h"
#include "print.h"
#include <stdint.h>

// Primary ATA channel ports
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_SECCOUNT    0x1F2
#define ATA_PRIMARY_LBA_LO      0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HI      0x1F5
#define ATA_PRIMARY_DRIVE_SEL   0x1F6
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_CMD         0x1F7
#define ATA_PRIMARY_CTRL        0x3F6

// Status register bits
#define ATA_SR_BSY   0x80   // Busy
#define ATA_SR_DRDY  0x40   // Drive ready
#define ATA_SR_DRQ   0x08   // Data request
#define ATA_SR_ERR   0x01   // Error

// Commands
#define ATA_CMD_READ_PIO   0x20
#define ATA_CMD_WRITE_PIO  0x30
#define ATA_CMD_IDENTIFY   0xEC

// Drive selector values (LBA mode bit set)
#define ATA_SEL_MASTER  0xE0
#define ATA_SEL_SLAVE   0xF0

static uint32_t drive_sectors[2] = {0, 0};

// 400ns delay - read status port 4 times (each ~100ns on real hardware)
static void ata_delay(void) {
    port_inb(ATA_PRIMARY_CTRL);
    port_inb(ATA_PRIMARY_CTRL);
    port_inb(ATA_PRIMARY_CTRL);
    port_inb(ATA_PRIMARY_CTRL);
}

// Wait until BSY clears, then return status.
// Returns 0xFF on timeout (drive hung).
static uint8_t ata_wait_bsy(void) {
    int timeout = 100000;
    uint8_t status;
    do {
        status = port_inb(ATA_PRIMARY_STATUS);
        if (--timeout == 0) return 0xFF;
    } while (status & ATA_SR_BSY);
    return status;
}

// Wait for DRQ (data ready) or ERR.
static uint8_t ata_wait_drq(void) {
    int timeout = 100000;
    uint8_t status;
    do {
        status = port_inb(ATA_PRIMARY_STATUS);
        if (status & ATA_SR_ERR) return status;
        if (--timeout == 0) return 0xFF;
    } while (!(status & ATA_SR_DRQ));
    return status;
}

static void ata_select_drive(uint8_t drive) {
    port_outb(ATA_PRIMARY_DRIVE_SEL, drive == ATA_DRIVE_MASTER ? 0xA0 : 0xB0);
    ata_delay();
}

bool ata_init(void) {
    // Disable interrupts on primary channel
    port_outb(ATA_PRIMARY_CTRL, 0x02);

    for (uint8_t d = 0; d < 2; d++) {


        ata_select_drive(d);
        ata_wait_bsy();

        // Send IDENTIFY
        port_outb(ATA_PRIMARY_SECCOUNT, 0);
        port_outb(ATA_PRIMARY_LBA_LO, 0);
        port_outb(ATA_PRIMARY_LBA_MID, 0);
        port_outb(ATA_PRIMARY_LBA_HI, 0);
        port_outb(ATA_PRIMARY_CMD, ATA_CMD_IDENTIFY);
        ata_delay();

        uint8_t status = port_inb(ATA_PRIMARY_STATUS);
        if (status == 0) { print_str("ata: no drive\n"); continue; }

        if (ata_wait_bsy() == 0xFF) { print_str("ata: BSY timeout\n"); continue; }

        uint8_t lba_mid = port_inb(ATA_PRIMARY_LBA_MID);
        uint8_t lba_hi  = port_inb(ATA_PRIMARY_LBA_HI);

        if (lba_mid || lba_hi) { print_str("ata: ATAPI, skipping\n"); continue; }

        uint8_t drq = ata_wait_drq();
        if (drq & ATA_SR_ERR) { print_str("ata: DRQ error\n"); continue; }

        // Read 256 words of IDENTIFY data
        uint16_t identify[256];
        for (int i = 0; i < 256; i++)
            identify[i] = port_inw(ATA_PRIMARY_DATA);

        drive_sectors[d] = ((uint32_t)identify[61] << 16) | identify[60];
        print_str("ata: drive ");
        print_char('0' + d);
        print_str(" sectors=");
        // print low 16 bits of sector count as hex
        uint32_t sc = drive_sectors[d];
        for (int i = 28; i >= 0; i -= 4) {
            uint8_t nibble = (sc >> i) & 0xF;
            print_char(nibble < 10 ? '0' + nibble : 'a' + nibble - 10);
        }
        print_char('\n');
    }

    return drive_sectors[ATA_DRIVE_MASTER] > 0;
}

bool ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint8_t* buf) {
    if (!count || drive > 1 || !drive_sectors[drive]) return false;

    ata_wait_bsy();

    uint8_t sel = (drive == ATA_DRIVE_MASTER ? ATA_SEL_MASTER : ATA_SEL_SLAVE);
    port_outb(ATA_PRIMARY_DRIVE_SEL, sel | ((lba >> 24) & 0x0F));
    ata_delay();
    ata_wait_bsy();

    port_outb(ATA_PRIMARY_ERROR,   0x00);
    port_outb(ATA_PRIMARY_SECCOUNT, count);
    port_outb(ATA_PRIMARY_LBA_LO,  (uint8_t)(lba));
    port_outb(ATA_PRIMARY_LBA_MID, (uint8_t)(lba >> 8));
    port_outb(ATA_PRIMARY_LBA_HI,  (uint8_t)(lba >> 16));
    port_outb(ATA_PRIMARY_CMD,     ATA_CMD_READ_PIO);

    for (uint8_t s = 0; s < count; s++) {
        uint8_t status = ata_wait_bsy();
        if (status == 0xFF || (status & ATA_SR_ERR)) return false;
        if (ata_wait_drq() & ATA_SR_ERR) return false;

        uint16_t* wbuf = (uint16_t*)(buf + s * 512);
        for (int i = 0; i < 256; i++)
            wbuf[i] = port_inw(ATA_PRIMARY_DATA);
        ata_delay();
    }
    return true;
}

bool ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, const uint8_t* buf) {
    if (!count || drive > 1 || !drive_sectors[drive]) return false;

    ata_wait_bsy();

    uint8_t sel = (drive == ATA_DRIVE_MASTER ? ATA_SEL_MASTER : ATA_SEL_SLAVE);
    port_outb(ATA_PRIMARY_DRIVE_SEL, sel | ((lba >> 24) & 0x0F));
    ata_delay();
    ata_wait_bsy();

    port_outb(ATA_PRIMARY_ERROR,    0x00);
    port_outb(ATA_PRIMARY_SECCOUNT, count);
    port_outb(ATA_PRIMARY_LBA_LO,   (uint8_t)(lba));
    port_outb(ATA_PRIMARY_LBA_MID,  (uint8_t)(lba >> 8));
    port_outb(ATA_PRIMARY_LBA_HI,   (uint8_t)(lba >> 16));
    port_outb(ATA_PRIMARY_CMD,      ATA_CMD_WRITE_PIO);

    for (uint8_t s = 0; s < count; s++) {
        uint8_t status = ata_wait_bsy();
        if (status == 0xFF || (status & ATA_SR_ERR)) return false;
        if (ata_wait_drq() & ATA_SR_ERR) return false;

        const uint16_t* wbuf = (const uint16_t*)(buf + s * 512);
        for (int i = 0; i < 256; i++)
            port_outw(ATA_PRIMARY_DATA, wbuf[i]);
        ata_delay();
    }

    // Flush write cache
    port_outb(ATA_PRIMARY_CMD, 0xE7);
    ata_wait_bsy();
    return true;
}

uint32_t ata_sector_count(uint8_t drive) {
    if (drive > 1) return 0;
    return drive_sectors[drive];
}