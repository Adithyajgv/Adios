#pragma once

#include <stddef.h>

int strcmp(const char* a, const char* b);
size_t strlen(const char* s);
void* memset(void* dest, int val, size_t len);
char* strcat(char* dest, const char* src);