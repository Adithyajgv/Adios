#include "x86_64/terminal.h"
#include "print.h"
#include "keyboard.h"
#include "string.h"
#include "x86_64/keycodes.h"
#include "bool.h"
#include "stdlib.h"
#include "power.h"
#include "vfs.h"
#include "x86_64/ata.h"

// FatFs for src command file reading
#include "ff.h"

#include <stddef.h>

#define INPUT_BUFFER_SIZE 128
#define VAR_EXPANSION_LIMIT 16

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

static const char* var_get_depth(const char* name, int depth) {
    if (depth > VAR_EXPANSION_LIMIT) return NULL;  // Prevent infinite recursion/cycles
    VarNode* cur = vars_head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            // If the value is itself a variable reference, expand recursively
            if (cur->value && cur->value[0] == '!')
                return var_get_depth(cur->value + 1, depth + 1);
            return cur->value;
        }
        cur = cur->next;
    }
    return NULL;
}

static const char* var_get(const char* name) {
    return var_get_depth(name, 0);
}



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
            if (val[0] == '!') {
                const char* expanded = var_get(val + 1);
                if (expanded)
                    var_set(varname, expanded);
                else
                    var_set(varname, val);
            } else {
                var_set(varname, val);
            }
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

    // --- Hardcoded built-in commands ---

    if (strcmp(argv[0], "echo") == 0) {
        for (int i = 1; i < argc; i++) {
            print_str(argv[i]);
            if (i + 1 < argc) print_char(' ');
        }
        print_char('\n');
        print_str(header);
        return;
    }

    if (strcmp(argv[0], "clear") == 0) {
        print_clear();
        print_str(header);
        return;
    }

    if (strcmp(argv[0], "info") == 0) {
        print_str("AdiOS v1.0\n");
        print_str(header);
        return;
    }

    if (strcmp(argv[0], "shutdown") == 0) {
        free_all();
        print_str("System is shutting down...\n");
        shutdown_system();
        return;
    }

    if (strcmp(argv[0], "help") == 0) {
        print_str("Built-in commands:\n");
        print_str("  echo, clear, info, shutdown, help\n");
        print_str("  mount [<drive> <path>]\n");
        print_str("  umount <path>\n");
        print_str("  src <file>\n");
        print_str("Other commands are looked up in /bin on the mounted filesystem.\n");
        print_str(header);
        return;
    }

    // mount <drive_num> <mount_point>
    // e.g.  mount 0 /
    if (strcmp(argv[0], "mount") == 0) {
        if (argc == 1) {
            // No args: list current mounts
            vfs_list_mounts();
        } else if (argc == 3) {
            int drv = argv[1][0] - '0';
            if (drv < 0 || drv > 1) {
                print_str("mount: drive must be 0 or 1\n");
            } else {
                if (vfs_mount(drv, argv[2]))
                    print_str("mounted\n");
            }
        } else {
            print_str("usage: mount [<drive> <path>]\n");
        }
        print_str(header);
        return;
    }

    // umount <mount_point>
    if (strcmp(argv[0], "umount") == 0) {
        if (argc != 2) {
            print_str("usage: umount <path>\n");
        } else {
            if (vfs_umount(argv[1]))
                print_str("unmounted\n");
        }
        print_str(header);
        return;
    }

    // src <file>  — execute each line of a file as a command (like bash source)
    if (strcmp(argv[0], "src") == 0) {
        if (argc != 2) {
            print_str("usage: src <file>\n");
            print_str(header);
            return;
        }
        char fat_path[VFS_MAX_PATH];
        if (!vfs_resolve(argv[1], fat_path)) {
            print_str("src: no filesystem mounted for ");
            print_str(argv[1]);
            print_char('\n');
            print_str(header);
            return;
        }
        FIL f;
        FRESULT res = f_open(&f, fat_path, FA_READ);
        if (res != FR_OK) {
            print_str("src: cannot open ");
            print_str(argv[1]);
            print_char('\n');
            print_str(header);
            return;
        }
        static char src_line[INPUT_BUFFER_SIZE];
        while (f_gets(src_line, sizeof(src_line), &f)) {
            // Strip trailing newline/CR
            size_t len = strlen(src_line);
            while (len > 0 && (src_line[len-1] == '\n' || src_line[len-1] == '\r'))
                src_line[--len] = '\0';
            if (len == 0) continue;
            print_str(src_line);
            print_char('\n');
            handle_command(src_line);
        }
        f_close(&f);
        print_str(header);
        return;
    }

    // --- Look up command in /bin on the mounted filesystem ---
    {
        // Try to find /bin/<cmd> on any mounted filesystem
        char bin_path[VFS_MAX_PATH];
        // Build the logical path  /bin/<argv[0]>
        char vfs_path[VFS_MAX_PATH];
        strcpy(vfs_path, "/bin/");
        size_t off = strlen(vfs_path);
        size_t clen = strlen(argv[0]);
        if (off + clen < VFS_MAX_PATH) {
            memcpy(vfs_path + off, argv[0], clen + 1);
        }

        if (vfs_resolve(vfs_path, bin_path)) {
            // File exists on disk — for now, report it as "not yet executable"
            // (full ELF loading is a future step; this scaffolding is where it goes)
            FIL probe;
            if (f_open(&probe, bin_path, FA_READ) == FR_OK) {
                f_close(&probe);
                print_str(argv[0]);
                print_str(": found in /bin but execution not yet supported\n");
                print_str(header);
                return;
            }
        }

        print_str("command not found: ");
        print_str(argv[0]);
        print_char('\n');
    }
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

void terminal_init(void) {
    // Bring up storage and filesystem layers
    ata_init();
    vfs_init();

    print_str("\nAdiOS Terminal\n");
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