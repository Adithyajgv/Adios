#include <stdint.h>
#include <stdbool.h>
#include "print.h"
#include "vfs.h"
#include "ff.h"

extern char keyboard_get_last_char();

// The File Descriptor Table
#define MAX_FDS 16
FIL file_objects[MAX_FDS];
bool fd_in_use[MAX_FDS] = {false};

int sys_file_open(const char* path, int mode) {
    char fatfs_path[256];
    if (!vfs_resolve(path, fatfs_path)) return -1; // VFS couldn't find the mount

    // Find a free FD slot
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (!fd_in_use[i]) { fd = i; break; }
    }
    if (fd == -1) return -1; // Too many open files

    BYTE ff_mode = (mode == 0) ? FA_READ : (FA_WRITE | FA_OPEN_ALWAYS);

    if (f_open(&file_objects[fd], fatfs_path, ff_mode) != FR_OK) return -1;

    fd_in_use[fd] = true;
    return fd;
}

int sys_file_read(int fd, void* buf, uint64_t size) {
    if (fd < 0 || fd >= MAX_FDS || !fd_in_use[fd]) return -1;
    UINT bytes_read;
    if (f_read(&file_objects[fd], buf, size, &bytes_read) != FR_OK) return -1;
    return bytes_read;
}

int sys_file_write(int fd, const void* buf, uint64_t size) {
    if (fd < 0 || fd >= MAX_FDS || !fd_in_use[fd]) return -1;
    UINT bytes_written;
    if (f_write(&file_objects[fd], buf, size, &bytes_written) != FR_OK) return -1;
    return bytes_written;
}

void sys_file_close(int fd) {
    if (fd >= 0 && fd < MAX_FDS && fd_in_use[fd]) {
        f_close(&file_objects[fd]);
        fd_in_use[fd] = false;
    }
}

int sys_file_create(const char* path) {
    char fatfs_path[256];
    if (!vfs_resolve(path, fatfs_path)) return -1;
    
    FIL temp;
    if (f_open(&temp, fatfs_path, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) return -1;
    f_close(&temp);
    return 0;
}



uint64_t syscall_handler(uint64_t id, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    //traps
    if (id == 1) {
        print_char((char)arg1);
    } 
    else if (id == 2) {
        print_str((const char*)arg1);
    }
    // Filesystem Syscalls
    else if (id == 3){
        return (uint64_t)sys_file_open((const char*)arg1, (int)arg2);
    }
    else if (id == 4){
        return (uint64_t)sys_file_read((int)arg1, (void*)arg2, arg3);
    }
    else if (id == 5){
        return (uint64_t)sys_file_write((int)arg1, (const void*)arg2, arg3);
    }
    else if (id == 6) { 
        sys_file_close((int)arg1); return 0; 
    }
    if (id == 7){
        return (uint64_t)sys_file_create((const char*)arg1);
    }
    else if (id == 8) {
        char c = 0;
        while ((c = keyboard_get_last_char()) == 0) {
            __asm__ volatile ("sti; hlt");
        }
        return (uint64_t)c;
    }
    else if (id == 9) {
        return (uint64_t)keyboard_get_last_char();
    }
}