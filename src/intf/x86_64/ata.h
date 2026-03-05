#pragma once

#include <stdint.h>
#include "bool.h"

// ATA drive identifiers
#define ATA_DRIVE_MASTER  0
#define ATA_DRIVE_SLAVE   1

// Initialise ATA subsystem. Returns true if primary master is present.
bool ata_init(void);

// Read `count` 512-byte sectors starting at `lba` from `drive` into `buf`.
// Returns true on success.
bool ata_read_sectors(uint8_t drive, uint32_t lba, uint8_t count, uint8_t* buf);

// Write `count` 512-byte sectors starting at `lba` to `drive` from `buf`.
// Returns true on success.
bool ata_write_sectors(uint8_t drive, uint32_t lba, uint8_t count, const uint8_t* buf);

// Return total sector count of `drive`, or 0 if unavailable.
uint32_t ata_sector_count(uint8_t drive);