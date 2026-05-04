#include <stdio.h>
#include <stdint.h>

// Note: print_char will eventually be a 'syscall' 
// defined in an assembly file or arch-specific C file.
__attribute__((weak)) void print_char(char c) {
    asm volatile (
        "mov $1, %%rax\n"   
        "mov %0, %%rdi\n"   
        "int $0x80"
        : 
        : "r"((uint64_t)c)
        : "rax", "rdi"
    );
}

char getchar(void) {
    uint64_t ret;
    asm volatile (
        "mov $8, %%rax\n"
        "int $0x80\n"
        "mov %%rax, %0\n"
        : "=r"(ret)
        :
        : "rax", "memory"
    );
    return (char)ret;
}

void print_string(const char* str) {
    asm volatile (
        "mov $2, %%rax\n"   // Let's say Syscall ID 2 is print_str
        "mov %0, %%rdi\n"   // Pass the pointer to the string in RDI
        "int $0x80"
        : 
        : "r"(str) 
        : "rax", "rdi", "memory"
    );
}

void print_int(int num) {
    char buffer[16];
    int i = 0;
    int is_negative = 0;

    if (num == 0) {
        print_string("0");
        return;
    }

    if (num < 0) {
        is_negative = 1;
        num = -num;
    }

    while (num > 0) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (is_negative) buffer[i++] = '-';

    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    buffer[i] = '\n';
    buffer[i+1] = '\0';
    print_string(buffer);
}

int file_open(const char* path, int mode) {
    int ret;
    asm volatile (
        "mov $3, %%rax\n"
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"(path), "r"((uint64_t)mode)
        : "rax", "rdi", "rsi", "memory"
    );
    return ret;
}

int file_read(int fd, void* buf, uint64_t size) {
    int ret;
    asm volatile (
        "mov $4, %%rax\n"
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "mov %3, %%rdx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint64_t)fd), "r"(buf), "r"(size)
        : "rax", "rdi", "rsi", "rdx", "memory"
    );
    return ret;
}

int file_write(int fd, const void* buf, uint64_t size) {
    int ret;
    asm volatile (
        "mov $5, %%rax\n"
        "mov %1, %%rdi\n"
        "mov %2, %%rsi\n"
        "mov %3, %%rdx\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"((uint64_t)fd), "r"(buf), "r"(size)
        : "rax", "rdi", "rsi", "rdx", "memory"
    );
    return ret;
}

void file_close(int fd) {
    asm volatile (
        "mov $6, %%rax\n"
        "mov %0, %%rdi\n"
        "int $0x80\n"
        : 
        : "r"((uint64_t)fd)
        : "rax", "rdi"
    );
}

int file_create(const char* path) {
    int ret;
    asm volatile (
        "mov $7, %%rax\n"
        "mov %1, %%rdi\n"
        "int $0x80\n"
        "mov %%eax, %0\n"
        : "=r"(ret)
        : "r"(path)
        : "rax", "rdi", "memory"
    );
    return ret;
}

char check_key(void) {
    uint64_t ret;
    asm volatile (
        "mov $8, %%rax\n"
        "int $0x80\n"
        "mov %%rax, %0\n"
        : "=r"(ret) 
        : 
        : "rax", "memory");
    return (char)ret;
}