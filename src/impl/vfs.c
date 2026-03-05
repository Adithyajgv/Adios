#include "vfs.h"
#include "print.h"
#include "string.h"
#include "stdlib.h"

// FatFs headers live in include/fatfs/
#include "ff.h"

typedef struct {
    int    drive;                    // ATA drive number (0 or 1)
    char   mount_pt[VFS_MAX_PATH];   // e.g. "/"
    FATFS  fs;                       // FatFs work area
    bool   active;
} VfsMount;

static VfsMount mount_table[VFS_MAX_MOUNTS];

void vfs_init(void) {
    for (int i = 0; i < VFS_MAX_MOUNTS; i++)
        mount_table[i].active = false;
}

bool vfs_mount(int drive, const char* mount_pt) {
    if (!mount_pt || drive < 0 || drive > 1) return false;

    // Check not already mounted at this point
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (mount_table[i].active && strcmp(mount_table[i].mount_pt, mount_pt) == 0) {
            print_str("vfs: already mounted at ");
            print_str(mount_pt);
            print_char('\n');
            return false;
        }
    }

    // Find a free slot
    int slot = -1;
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (!mount_table[i].active) { slot = i; break; }
    }
    if (slot < 0) { print_str("vfs: mount table full\n"); return false; }

    // Build FatFs logical drive string, e.g. "0:"
    char drv_str[4];
    drv_str[0] = '0' + drive;
    drv_str[1] = ':';
    drv_str[2] = '\0';

    FRESULT res = f_mount(&mount_table[slot].fs, drv_str, 1 /* mount now */);
    if (res != FR_OK) {
        print_str("vfs: f_mount failed, FRESULT=");
        // Print up to 2 digits
        if (res >= 10) print_char('0' + (res / 10));
        print_char('0' + (res % 10));
        print_char('\n');
        return false;
    }

    mount_table[slot].drive  = drive;
    mount_table[slot].active = true;
    size_t len = strlen(mount_pt);
    if (len >= VFS_MAX_PATH) len = VFS_MAX_PATH - 1;
    memcpy(mount_table[slot].mount_pt, mount_pt, len + 1);

    return true;
}

bool vfs_umount(const char* mount_pt) {
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (!mount_table[i].active) continue;
        if (strcmp(mount_table[i].mount_pt, mount_pt) != 0) continue;

        char drv_str[4];
        drv_str[0] = '0' + mount_table[i].drive;
        drv_str[1] = ':';
        drv_str[2] = '\0';
        f_mount(NULL, drv_str, 0);  // Unmount

        mount_table[i].active = false;
        return true;
    }
    print_str("vfs: no filesystem mounted at ");
    print_str(mount_pt);
    print_char('\n');
    return false;
}

bool vfs_resolve(const char* path, char* out) {
    if (!path || !out) return false;

    // Find the most specific (longest) matching mount point
    int best = -1;
    size_t best_len = 0;

    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (!mount_table[i].active) continue;
        const char* mp = mount_table[i].mount_pt;
        size_t mp_len = strlen(mp);
        if (mp_len > best_len && strncmp(path, mp, mp_len) == 0) {
            // Ensure we match a full path component
            if (path[mp_len] == '/' || path[mp_len] == '\0' || mp[mp_len-1] == '/') {
                best     = i;
                best_len = mp_len;
            }
        }
    }

    if (best < 0) return false;

    // Build FatFs path: "<drive>:<remainder>"
    // e.g. path="/bin/ls", mount_pt="/", remainder="bin/ls"
    out[0] = '0' + mount_table[best].drive;
    out[1] = ':';
    out[2] = '/';

    const char* remainder = path + best_len;
    while (*remainder == '/') remainder++;  // strip leading slashes

    size_t rlen = strlen(remainder);
    if (2 + 1 + rlen >= VFS_MAX_PATH) rlen = VFS_MAX_PATH - 4;
    memcpy(out + 3, remainder, rlen + 1);

    return true;
}

void vfs_list_mounts(void) {
    bool any = false;
    for (int i = 0; i < VFS_MAX_MOUNTS; i++) {
        if (!mount_table[i].active) continue;
        print_char('0' + mount_table[i].drive);
        print_str(": -> ");
        print_str(mount_table[i].mount_pt);
        print_char('\n');
        any = true;
    }
    if (!any) print_str("(no filesystems mounted)\n");
}