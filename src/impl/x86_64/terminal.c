#include "x86_64/terminal.h"
#include "print.h"
#include "keyboard.h"
#include "string.h"
#include "x86_64/keycodes.h"

#define INPUT_BUFFER_SIZE 128

static char *user = "adi";
static char *host = "AdiosOS";
static char *header = "";

static char input_buffer[INPUT_BUFFER_SIZE];
static size_t buffer_pos = 0;

static void handle_command(const char* cmd) {
    if (strcmp(cmd, "hello") == 0) {
        print_str("\nHello, world!\n");
    } else if (strcmp(cmd, "info") == 0) {
        print_str("\nAdiOS v0.0.1\n");
    } else if (strcmp(cmd, "clear") == 0) {
        print_clear();
    } else if (strlen(cmd) == 0) {
        print_char('\n');
    } else {
        print_str("\nUnknown command: ");
        print_str((char*)cmd);
        print_char('\n');
    }
}

static void terminal_put_char(char c) {
    if (c == '\r') {
        c = '\n';
    }
    
    print_char(c);
}

static void terminal_handle_input(struct KeyboardEvent event) {
    if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
        char c = to_ascii(event.code);
        if (c == '\n') {
            input_buffer[buffer_pos] = '\0';
            handle_command(input_buffer);
            buffer_pos = 0;
            print_str(header);
        } else if (c == '\b' || event.code == KEY_CODE_BACKSPACE) {
            if (buffer_pos > 0) {
                buffer_pos--;
                backspace();
            }
        } else {
            if (buffer_pos < INPUT_BUFFER_SIZE - 1) {
                input_buffer[buffer_pos++] = c;
                terminal_put_char(c);
            }
        }
    }
}

void terminal_init(void) {
    print_str("\nAdiOS Terminal\n");
    strcat(header, user);
    strcat(header, "@");
    strcat(header, host);
    strcat(header, ":~$ ");
    print_str(header);
    keyboard_set_handler(terminal_handle_input);
}

void terminal_run(void) {
    // No polling needed since keyboard input is interrupt-driven
    // Simply keep main looping or running other tasks here.
}
