#include "stdlib.h"
#include "bool.h"
#include <stddef.h>
#include <stdint.h>

/*
 * Kernel heap allocator -- ported from CS354 p3Heap.
 * Best-fit placement, immediate coalescing, p-bit tracking.
 *
 * Backing memory comes from the linker script symbols
 * _heap_start and _heap_end instead of mmap().
 */

typedef struct blockHeader {
    int size_status;
} blockHeader;

/* Linker script symbols */
extern char _heap_start;
extern char _heap_end;

static blockHeader *heap_start_ptr = NULL;
static int heap_total_size         = 0;
static bool initialized            = false;

void malloc_init(void) {
    heap_total_size = (int)((uintptr_t)&_heap_end - (uintptr_t)&_heap_start);

    /* Reserve 4 bytes at the start for double-word alignment (same as CS354) */
    heap_start_ptr = (blockHeader*)((void*)&_heap_start + 4);

    /* Total usable size: subtract the 4-byte pad and 4-byte end mark */
    int usable = heap_total_size - 8;

    /* One large free block covering all usable space */
    heap_start_ptr->size_status = usable + 2; /* p-bit set: treat pad as allocated */

    /* Footer for the initial free block */
    blockHeader *footer = (blockHeader*)((void*)heap_start_ptr + usable - sizeof(blockHeader));
    footer->size_status = usable;

    /* End mark */
    blockHeader *end_mark = (blockHeader*)((void*)heap_start_ptr + usable);
    end_mark->size_status = 1;

    initialized = true;
}

bool malloc_is_initialized(void) {
    return initialized;
}

void* malloc(size_t size) {
    if (!initialized) malloc_init();
    if (size < 1) return NULL;

    /* Round up to multiple of 8 */
    int b_size = sizeof(blockHeader) + (int)size;
    if (b_size % 8 != 0)
        b_size += 8 - (b_size % 8);

    blockHeader *best_loc = NULL;
    blockHeader *curr     = heap_start_ptr;
    int found             = 0;

    while (!found && curr->size_status != 1) {
        int curr_size = curr->size_status - (curr->size_status % 8);

        if (curr->size_status % 2 != 0) {
            /* Allocated block -- skip */
        } else if (b_size == curr_size) {
            /* Exact fit */
            found    = 1;
            best_loc = curr;
        } else if (curr_size > b_size) {
            /* Larger -- keep if best so far */
            if (!best_loc ||
                (best_loc->size_status - (best_loc->size_status % 8)) > curr_size) {
                best_loc = curr;
            }
        }

        curr = (blockHeader*)((void*)curr + curr_size);
    }

    if (found) {
        /* Exact fit -- just mark allocated */
        best_loc->size_status += 1;
        blockHeader *next = (blockHeader*)((void*)best_loc +
                            (best_loc->size_status - (best_loc->size_status % 8)));
        if (next->size_status != 1)
            next->size_status += 2;
        return (void*)(best_loc + 1);
    }

    if (best_loc) {
        int orig_size = best_loc->size_status - (best_loc->size_status % 8);

        if (orig_size - b_size >= 8) {
            /* Split the block */
            blockHeader *new_hdr = (blockHeader*)((void*)best_loc + b_size);
            new_hdr->size_status = (orig_size - b_size) + 2;

            blockHeader *new_footer = (blockHeader*)((void*)new_hdr +
                                      (new_hdr->size_status - (new_hdr->size_status % 8)) -
                                      sizeof(blockHeader));
            new_footer->size_status = new_hdr->size_status - (new_hdr->size_status % 8);

            best_loc->size_status = (best_loc->size_status & 2) + b_size + 1;
        } else {
            /* Not worth splitting -- allocate whole block */
            best_loc->size_status += 1;
            blockHeader *next = (blockHeader*)((void*)best_loc +
                                (best_loc->size_status - (best_loc->size_status % 8)));
            if (next->size_status != 1)
                next->size_status += 2;
        }

        return (void*)(best_loc + 1);
    }

    return NULL;
}

void free(void *ptr) {
    if (ptr == NULL) return;
    if ((uintptr_t)ptr % 8 != 0) return;

    void *heap_end = (void*)heap_start_ptr + heap_total_size - 8;

    blockHeader *header = (blockHeader*)ptr - 1;
    if ((void*)header < (void*)heap_start_ptr || (void*)header >= heap_end) return;
    if (header->size_status % 2 == 0) return; /* Already free */

    /* Mark free */
    header->size_status -= 1;
    int block_size = header->size_status - (header->size_status % 8);

    /* Write footer */
    blockHeader *footer = (blockHeader*)((void*)header + block_size - sizeof(blockHeader));
    footer->size_status = block_size;

    /* Coalesce with next block if free */
    blockHeader *next = (blockHeader*)((void*)header + block_size);
    int next_size = 0;
    if (next->size_status != 1) {
        if (next->size_status % 2 != 0) {
            /* Next is allocated -- clear its p-bit */
            next->size_status -= 2;
        } else {
            /* Next is free -- merge */
            next_size = next->size_status - (next->size_status % 8);
            header->size_status += next_size;
            footer = (blockHeader*)((void*)header + block_size + next_size - sizeof(blockHeader));
            footer->size_status = block_size + next_size;
        }
    }

    /* Coalesce with previous block if free (p-bit == 0) */
    if ((header->size_status % 8) < 2) {
        int prev_size = (header - 1)->size_status;
        blockHeader *prev_hdr = (blockHeader*)((void*)header - prev_size);
        prev_hdr->size_status += block_size + next_size;
        footer->size_status   += prev_size;
    }
}

void free_all(void) {
    /* Re-initialise the heap from scratch -- used on shutdown */
    initialized = false;
    malloc_init();
}