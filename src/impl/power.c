#include "io.h"
#include "power.h"

#define PM1a_CNT 0x604
#define SLP_TYP_SHIFT 10
#define SLP_EN (1 << 13)

void shutdown_system(void) {
    uint16_t val = (0x1 << SLP_TYP_SHIFT) | SLP_EN;
    outw(PM1a_CNT, val);
    // If shutdown fails, halt CPU
    while (1) {
        __asm__ volatile ("hlt");
    }
}
