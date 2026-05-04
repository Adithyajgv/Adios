#include <stdint.h>
#include <stdbool.h>
#include <stdio.h> 
#include "stdlib.h" // Your new p3Heap allocator!


void print_str(const char* str) { print_string(str); }
void print_num(int num) { print_int(num); }

extern int file_open(const char* path, int mode);
extern int file_read(int fd, void* buf, uint64_t size);
extern int file_write(int fd, const void* buf, uint64_t size);
extern void file_close(int fd);
extern int file_create(const char* path);
extern char getchar(void); 
extern void print_char(char c);
extern void print_string(const char* str);
extern void print_int(int num);
extern char check_key(void);

#define MAX_LINES 1000
#define MAX_LINE_LENGTH 256
#define BUFFER_SIZE (MAX_LINES * MAX_LINE_LENGTH)

bool editor_running = true;

typedef struct {
    char* lines[MAX_LINES];
    int line_count;
    char* filename;
    bool modified;
} TextBuffer;

TextBuffer buffer;

int str_len(const char* str) {
    int len = 0;
    while (str[len]) len++;
    return len;
}

void str_copy(char* dest, const char* src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
}

int str_to_int(const char* str) {
    int result = 0;
    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        str++;
    }
    return result;
}

void read_line(char* buf, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        char c = getchar();
        if (c == 0) {
            continue; 
        }

        if (c == '\n' || c == '\r') {
            break;
        } else if (c == '\b' || c == 127) {
            if (i > 0) {
                i--;
                print_str("\b \b"); 
            }
        } else if (c >= 32 && c < 127) {
            buf[i++] = c;
            //print_char(c);
        }
    }
    buf[i] = '\0';
    print_char('\n');
}

void init_buffer(const char* filename) {
    buffer.filename = (char*)filename;
    buffer.line_count = 0;
    buffer.modified = false;
    for (int i = 0; i < MAX_LINES; i++) buffer.lines[i] = NULL;
}

bool load_file(const char* filename) {
    int fd = file_open(filename, 0); 
    if (fd < 0) {
        print_str("File doesn't exist. Creating new file.\n");
        file_create(filename);
        return true; 
    }
    
    char* file_buffer = (char*)malloc(BUFFER_SIZE);
    if (!file_buffer) {
        print_str("Error: Out of memory\n");
        file_close(fd);
        return false;
    }
    
    int bytes_read = file_read(fd, file_buffer, BUFFER_SIZE - 1);
    file_close(fd);
    
    if (bytes_read < 0) {
        print_str("Error reading file\n");
        return false;
    }
    
    file_buffer[bytes_read] = '\0';
    char* line_start = file_buffer;
    for (int i = 0; i < bytes_read && buffer.line_count < MAX_LINES; i++) {
        if (file_buffer[i] == '\n') {
            file_buffer[i] = '\0';
            int line_len = str_len(line_start);
            buffer.lines[buffer.line_count] = (char*)malloc(line_len + 1);
            str_copy(buffer.lines[buffer.line_count], line_start);
            buffer.line_count++;
            line_start = &file_buffer[i + 1];
        }
    }
    
    if (line_start < file_buffer + bytes_read) {
        int line_len = str_len(line_start);
        buffer.lines[buffer.line_count] = (char*)malloc(line_len + 1);
        str_copy(buffer.lines[buffer.line_count], line_start);
        buffer.line_count++;
    }
    
    free(file_buffer); // Freeing the massive read buffer!
    
    print_str("Loaded ");
    print_num(buffer.line_count);
    print_str(" lines\n");
    return true;
}

bool save_file() {
    file_create(buffer.filename);
    int fd = file_open(buffer.filename, 1);
    if (fd < 0) {
        print_str("Error: Cannot open file for writing\n");
        return false;
    }
    
    for (int i = 0; i < buffer.line_count; i++) {
        int len = str_len(buffer.lines[i]);
        file_write(fd, buffer.lines[i], len);
        file_write(fd, "\n", 1);
    }
    
    file_close(fd);
    buffer.modified = false;
    print_str("File saved.\n");
    return true;
}

void print_lines(int start, int end) {
    if (start < 1) start = 1;
    if (end > buffer.line_count) end = buffer.line_count;
    for (int i = start - 1; i < end; i++) {
        print_num(i + 1);
        print_str(": ");
        print_str(buffer.lines[i]);
        print_char('\n');
    }
}

void insert_line(int line_num, const char* text) {
    if (line_num < 1 || line_num > buffer.line_count + 1) return;
    if (buffer.line_count >= MAX_LINES) return;
    
    for (int i = buffer.line_count; i >= line_num; i--) {
        buffer.lines[i] = buffer.lines[i - 1];
    }
    
    int len = str_len(text);
    buffer.lines[line_num - 1] = (char*)malloc(len + 1);
    str_copy(buffer.lines[line_num - 1], text);
    buffer.line_count++;
    buffer.modified = true;
}

void delete_line(int line_num) {
    if (line_num < 1 || line_num > buffer.line_count) return;
    
    free(buffer.lines[line_num - 1]); // Freeing the deleted line!
    
    for (int i = line_num - 1; i < buffer.line_count - 1; i++) {
        buffer.lines[i] = buffer.lines[i + 1];
    }
    buffer.line_count--;
    buffer.modified = true;
}

void show_help() {
    print_str("\nATE Commands:\n");
    print_str("  i <line> <text>  - Insert text at line number\n");
    print_str("  a <text>         - Append text at end\n");
    print_str("  d <line>         - Delete line\n");
    print_str("  p                - Print all lines\n");
    print_str("  s                - Save file\n");
    print_str("  q                - Quit\n");
    print_str("  h                - Show this help\n\n");
}

void parse_command(char* cmd) {
    while (*cmd == ' ') cmd++;
    if (cmd[0] == '\0') return;
    
    if (cmd[0] == 'h') show_help();
    else if (cmd[0] == 'p') {
        print_lines(1, buffer.line_count);
    }
    else if (cmd[0] == 'i') {
        char* arg = cmd + 1;
        while (*arg == ' ') arg++;
        int line_num = str_to_int(arg);
        while (*arg >= '0' && *arg <= '9') arg++;
        while (*arg == ' ') arg++;
        insert_line(line_num, arg);
    }
    else if (cmd[0] == 'a') {
        char* text = cmd + 1;
        while (*text == ' ') text++;
        insert_line(buffer.line_count + 1, text);
    }
    else if (cmd[0] == 'd') {
        char* arg = cmd + 1;
        while (*arg == ' ') arg++;
        delete_line(str_to_int(arg));
    }
    else if (cmd[0] == 's') save_file();
    else if (cmd[0] == 'q') {
        print_str("Cleaning up memory...\n");
        // FIX: The ultimate memory leak patch! Free all loaded lines before halting.
        for (int i = 0; i < buffer.line_count; i++) {
            if (buffer.lines[i] != NULL) free(buffer.lines[i]);
        }
        print_str("Exiting ATE...\n");
        editor_running = false; 
    } else {
        print_str("Unknown command. Type 'h' for help.\n");
    }
}

void _start() {
    const char* filename = "/test.txt"; 
    
    print_str("ATE - AdiOS Text Editor\nEditing: ");
    print_str(filename);
    print_char('\n');
    
    init_buffer(filename);
    if (!load_file(filename)) return;
    
    show_help();
    
    char command[MAX_LINE_LENGTH];
    while (editor_running) {
        print_str("ate> ");
        read_line(command, MAX_LINE_LENGTH);
        parse_command(command);
    }
    return;
}