#include "x86_64/terminal.h"
#include "print.h"
#include "keyboard.h"
#include "string.h"
#include "x86_64/keycodes.h"
#include "bool.h"
#include "stdlib.h"
#include "power.h"

#include <stddef.h>

#define INPUT_BUFFER_SIZE 128

static char* user = "Adi";
static char* host = "AdiOS";

// Prompt header buffer
static char header[64] = {0};

// Simple key-value var store linked list
typedef struct VarNode {
    char* name;
    char* value;
    struct VarNode* next;
} VarNode;

static VarNode* vars_head = NULL;

static void var_set(const char* name, const char* value) {
    // Check if var exists, update
    VarNode* cur = vars_head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            free(cur->value);
            size_t len = strlen(value)+1;
            cur->value = malloc(len);
            memcpy(cur->value, value, len);
            return;
        }
        cur = cur->next;
    }
    // Add new var
    VarNode* new_var = malloc(sizeof(VarNode));
    if (!new_var) return;
    size_t nlen = strlen(name)+1;
    size_t vlen = strlen(value)+1;
    new_var->name = malloc(nlen);
    new_var->value = malloc(vlen);
    if (!new_var->name || !new_var->value) {
        free(new_var->name);
        free(new_var->value);
        free(new_var);
        return;
    }
    memcpy(new_var->name, name, nlen);
    memcpy(new_var->value, value, vlen);
    new_var->next = vars_head;
    vars_head = new_var;
}

static const char* var_get(const char* name) {
    VarNode* cur = vars_head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) return cur->value;
        cur = cur->next;
    }
    return NULL;
}

// Command function prototypes
static void cmd_echo(int argc, char** argv);
static void cmd_clear(int argc, char** argv);
static void cmd_info(int argc, char** argv);
static void cmd_shutdown(int argc, char** argv);
static void cmd_help(int argc, char** argv);

// Command list
typedef struct {
    const char* name;
    void (*func)(int argc, char** argv);
} Command;

static const Command commands[] = {
    { "echo", cmd_echo },
    { "clear", cmd_clear },
    { "info", cmd_info },
    { "help", cmd_help },
    { "shutdown", cmd_shutdown },
    { NULL, NULL }
};

// Input buffer
static char input_buffer[INPUT_BUFFER_SIZE];
static size_t buffer_pos = 0;

// Basic tokenizer - splits on spaces, no quote handling here
static char* simple_tokenize(char* str, char** saveptr) {
    char* p = str ? str : *saveptr;
    if (!p) return NULL;

    while (*p == ' ') p++;
    if (*p == '\0') {
        *saveptr = NULL;
        return NULL;
    }

    char* start = p;
    while (*p && *p != ' ') p++;
    if (*p) {
        *p = '\0';
        *saveptr = p + 1;
    } else {
        *saveptr = NULL;
    }
    return start;
}

static void handle_command(char* line) {
    // Check if it's a var declaration starting with !
    if (line[0] == '!') {
        // Format: !var="value" or !var=value
        char* eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char* varname = line + 1;  // skip '!'
            char* val = eq + 1;
            // If quoted, strip quotes
            size_t len = strlen(val);
            if (len >= 2 && val[0] == '"' && val[len-1] == '"') {
                val[len-1] = '\0';
                val++;
            }
            var_set(varname, val);
        } else {
            print_str("\nInvalid variable declaration. Usage: !var=\"value\"\n");
        }
        print_str(header);
        return;
    }

    // Parse arguments normally
    char* argv[16];
    int argc = 0;
    char* saveptr = NULL;
    for (char* token = simple_tokenize(line, &saveptr);
         token != NULL && argc < 16;
         token = simple_tokenize(NULL, &saveptr)) {
        // Expand variables if start with '!'
        if (token[0] == '!') {
            const char* val = var_get(token + 1);
            if (val) {
                argv[argc++] = (char*)val;  // Cast away const is safe here
                continue;
            }
        }
        argv[argc++] = token;
    }
    if (argc == 0) return;

    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcmp(argv[0], commands[i].name) == 0) {
            commands[i].func(argc, argv);
            print_str(header);
            return;
        }
    }

    print_str("\nUnknown command: ");
    print_str(argv[0]);
    print_char('\n');
    print_str(header);
}


static void print_backspace(void) {
    extern size_t col, row;
    if (col == 0) return;
    col--;
    struct Char blank = {' ', (uint8_t)(PRINT_COLOR_YELLOW | (PRINT_COLOR_BLACK << 4))};
    ((struct Char*)0xb8000)[col + 80 * row] = blank;
}

static void terminal_handle_input(struct KeyboardEvent event) {
    if (event.type != KEYBOARD_EVENT_TYPE_MAKE) return;

    char c = to_ascii(event.code, keyboard_is_shift_down());
    if (c == '\n') {
        input_buffer[buffer_pos] = '\0';
        print_char('\n');
        handle_command(input_buffer);
        buffer_pos = 0;
    } else if (c == '\b' || event.code == KEY_CODE_BACKSPACE) {
        if (buffer_pos > 0) {
            buffer_pos--;
            print_backspace();
        }
    } else {
        if (buffer_pos < INPUT_BUFFER_SIZE - 1) {
            input_buffer[buffer_pos++] = c;
            print_char(c);
        }
    }
}

// Command implementations
static void cmd_echo(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        print_str(argv[i]);
        if (i + 1 < argc) print_char(' ');
    }
    print_char('\n');
}
static void cmd_clear(int argc, char** argv) {
    (void)argc; (void)argv;
    print_clear();
}
static void cmd_info(int argc, char** argv) {
    (void)argc; (void)argv;
    print_str("AdiOS v1.0\n");
}

static void cmd_shutdown(int argc, char** argv) {
    (void)argc; (void)argv;
    free_all();
    print_str("System is shutting down...\n");
    shutdown_system();
}

static void cmd_help(int argc, char** argv) {
    (void)argc; (void)argv;
    print_str("Available commands:\n");
    for (int i = 0; commands[i].name != NULL; i++) {
        print_str("- ");
        print_str(commands[i].name);
        print_char('\n');
    }
}

void terminal_init(void) {
    print_str("\nSimple OS Terminal\n");
    // Build prompt header
    strcpy(header, user);
    strcat(header, "@");
    strcat(header, host);
    strcat(header, ":~$ ");
    print_str(header);
    keyboard_set_handler(terminal_handle_input);
}

void terminal_run(void) {
    // Input handled asynchronously, no polling needed here
}
