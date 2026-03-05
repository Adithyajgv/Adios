#pragma once

#include <stddef.h>

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t n);
size_t strlen(const char* s);
void* memset(void* dest, int val, size_t len);
int memcmp(const void* a, const void* b, size_t n);
char* strcat(char* dest, const char* src);
char* strcpy(char* dest, const char* src);
void* memcpy(void* dest, const void* src, size_t n);
char* strchr(const char* s, int c);