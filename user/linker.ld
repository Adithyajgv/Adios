OUTPUT_FORMAT(elf64-x86-64)
ENTRY(_start)

SECTIONS
{
    /*
     * A typical user-space base address.
     * This address must be chosen carefully to not conflict with your kernel's memory map.
     * For a simple test, 0x400000 is a common choice for user-space programs.
     * Your ELF loader loads segments to their virtual address (p_vaddr), so this will
     * be loaded into physical address 0x400000.
     */
    . = 0x400000;

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
        . = ALIGN(4096); /* Align BSS to page boundary */
    }
}
