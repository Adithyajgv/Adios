#include "print.h"

void syscall_handler(uint64_t id, uint64_t arg1) {
    if (id == 1) {
        print_char((char)arg1);
    } 
    else if (id == 2) {
        print_str((const char*)arg1);
    }
}