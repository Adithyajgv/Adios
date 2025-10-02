#include "stdlib.h"
#include "bool.h"


// Heap boundaries - define these in your linker script
extern char _heap_start;
extern char _heap_end;

#define MAX_ALLOCATIONS 1024

static uintptr_t heap_ptr = 0;
static void* alloc_ptrs[MAX_ALLOCATIONS];
static size_t alloc_count = 0;
static bool initialized = false;

void malloc_init() {
    heap_ptr = (uintptr_t)&_heap_start;
    alloc_count = 0;
    initialized = true;
}

bool malloc_is_initialized() {
    return initialized;
}

void* malloc(size_t size) {
    if (!initialized) malloc_init();

    // Align size to 8 bytes
    size = (size + 7) & (~7);

    uintptr_t current = heap_ptr;
    uintptr_t new_ptr = current + size;

    if (new_ptr >= (uintptr_t)&_heap_end) {
        // Out of heap memory
        return NULL;
    }

    heap_ptr = new_ptr;

    if (alloc_count < MAX_ALLOCATIONS) {
        alloc_ptrs[alloc_count++] = (void*)current;
    }

    return (void*)current;
}

void free(void* ptr) {
    if (ptr == NULL) return;

    // Remove pointer from tracking list by swapping with last pointer
    for (size_t i = 0; i < alloc_count; i++) {
        if (alloc_ptrs[i] == ptr) {
            alloc_ptrs[i] = alloc_ptrs[alloc_count - 1];
            alloc_count--;
            break;
        }
    }

    // Actual memory is not reclaimed (bump allocator), so free is a bookkeeping operation
}

void free_all(void) {
    // Reset bump allocator
    heap_ptr = (uintptr_t)&_heap_start;
    alloc_count = 0;
}
