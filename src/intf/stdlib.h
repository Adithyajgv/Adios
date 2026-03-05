#pragma once
#include <stddef.h>
#include "bool.h"

void* malloc(size_t size);
void free(void* ptr);
void free_all(void);
void malloc_init(void);
bool malloc_is_initialized(void);
