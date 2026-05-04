#pragma once

#ifndef STDLIB_H
#define STDLIB_H

#include <stdint.h>
#include <stddef.h>
#include "bool.h"

void malloc_init(void);
void* malloc(size_t size);
void free(void *ptr);
void free_all(void);
bool malloc_is_initialized(void);

#endif