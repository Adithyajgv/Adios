/*
 * FatFs diskio implementation for AdiOS.
 * Physical drive 0 = ATA primary master.
 * Physical drive 1 = ATA primary slave.
 *
 * This replaces the skeleton diskio.c shipped with FatFs.
 */

#include "ff.h"
#include "diskio.h"
#include "x86_64/ata.h"
#include "x86_64/rtc.h"

#define DEV_ATA0  0
#define DEV_ATA1  1

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv > DEV_ATA1) return STA_NOINIT;
    return (ata_sector_count(pdrv) > 0) ? 0 : STA_NOINIT;
}

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv > DEV_ATA1) return STA_NOINIT;
    // ATA is already initialised by ata_init() at boot;
    // just confirm the drive is present.
    return (ata_sector_count(pdrv) > 0) ? 0 : STA_NOINIT;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv > DEV_ATA1) return RES_PARERR;
    if (!count) return RES_PARERR;
    // ATA PIO handles up to 255 sectors per call; loop if needed.
    while (count > 0) {
        UINT batch = (count > 255) ? 255 : count;
        if (!ata_read_sectors(pdrv, (uint32_t)sector, (uint8_t)batch, buff))
            return RES_ERROR;
        buff   += batch * 512;
        sector += batch;
        count  -= batch;
    }
    return RES_OK;
}

#if FF_FS_READONLY == 0
DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv > DEV_ATA1) return RES_PARERR;
    if (!count) return RES_PARERR;
    while (count > 0) {
        UINT batch = (count > 255) ? 255 : count;
        if (!ata_write_sectors(pdrv, (uint32_t)sector, (uint8_t)batch, buff))
            return RES_ERROR;
        buff   += batch * 512;
        sector += batch;
        count  -= batch;
    }
    return RES_OK;
}
#endif

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv > DEV_ATA1) return RES_PARERR;

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;  // PIO writes are synchronous

        case GET_SECTOR_COUNT: {
            uint32_t n = ata_sector_count(pdrv);
            if (!n) return RES_ERROR;
            *(LBA_t*)buff = n;
            return RES_OK;
        }

        case GET_SECTOR_SIZE:
            *(WORD*)buff = 512;
            return RES_OK;

        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1;  // Not flash; erase block = 1 sector
            return RES_OK;

        default:
            return RES_PARERR;
    }
}

// FatFs timestamp callback — use RTC seconds as a rough clock.
DWORD get_fattime(void) {
    // Bits 31-25: year offset from 1980  (use 2024 = 44)
    // Bits 24-21: month  (1-12)
    // Bits 20-16: day    (1-31)
    // Bits 15-11: hour   (0-23)
    // Bits 10-5 : minute (0-59)
    // Bits 4-0  : second/2
    uint8_t secs = rtc_seconds();
    return ((DWORD)(2024 - 1980) << 25)
         | ((DWORD)1  << 21)
         | ((DWORD)1  << 16)
         | ((DWORD)0  << 11)
         | ((DWORD)0  << 5)
         | ((DWORD)(secs / 2));
}