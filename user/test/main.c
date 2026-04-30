#include <stdint.h>
#include <stdio.h>

// The entry point for the ELF executable
void _start() {
    volatile uint64_t counter = 0;
    while (1) {
        print_int(counter);
        counter++;
    }
}