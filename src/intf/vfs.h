#pragma once

#include "bool.h"
#include <stddef.h>

#define VFS_MAX_MOUNTS  4
#define VFS_MAX_PATH   128

/*
 * Mount a FAT filesystem.
 *   drive    : ATA drive number (0 = primary master, 1 = primary slave)
 *   mount_pt : absolute path to mount at, e.g. "/"
 * Returns true on success.
 */
bool vfs_mount(int drive, const char* mount_pt);

/*
 * Unmount the filesystem at `mount_pt`.
 * Returns true on success.
 */
bool vfs_umount(const char* mount_pt);

/*
 * Translate a VFS path (e.g. "/bin/ls") into a FatFs path (e.g. "0:/bin/ls").
 * Writes result into `out` (must be at least VFS_MAX_PATH bytes).
 * Returns true if a mount covers the path.
 */
bool vfs_resolve(const char* path, char* out);

/*
 * Print all active mount points to the terminal.
 */
void vfs_list_mounts(void);

/*
 * Initialise the VFS subsystem (zero the mount table).
 */
void vfs_init(void);