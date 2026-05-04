OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    /* Load user programs at 0x400000 */
    . = 0x400000;

    .text : ALIGN(4K) {
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(COMMON)
        *(.bss*)
    }

    /* --- ADD HEAP HERE --- */
    . = ALIGN(4K);
    _heap_start = .;
    
    /* Reserve 1 Megabyte for the user-space heap */
    . += 1M; 
    
    _heap_end = .;
}
