ENTRY(start)

SECTIONS
{
	. = 1M;

	.boot :
	{
		KEEP(*(.multiboot_header))
	}

	.text :
	{
		*(.text)
	}

	/* Align the end of .text to 4KB */
    . = ALIGN(0x1000);

    /* Define the start of the heap */
    _heap_start = .;

    /* Reserve 1MB for heap */
    .heap (NOLOAD) :
    {
        . = . + 1M;
    }

    /* Define the end of the heap */
    _heap_end = .;
}
