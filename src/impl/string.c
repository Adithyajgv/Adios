#include "string.h"

int strcmp(const char* a, const char* b) {
    while (*a && (*a == *b)) {
        a++; b++;
    }
    return *(unsigned char*)a - *(unsigned char*)b;
}

size_t strlen(const char* s) {
    size_t len = 0;
    while (s[len]) len++;
    return len;
}

char* strcat(char* dest, const char* src) {
    char* ptr = dest + strlen(dest);  // Find end of dest string
    while (*src) {
        *ptr++ = *src++;               // Copy from src to end of dest
    }
    *ptr = '\0';                      // Null-terminate
    return dest;
}

void* memset(void* dest, int val, size_t len) {
    unsigned char* ptr = (unsigned char*)dest;
    while (len-- > 0) {
        *ptr++ = (unsigned char)val;
    }
    return dest;
}