#include <stddef.h>
#include "keyboard.h"
#include "x86_64/idt.h"
#include "x86_64/ps2.h"

#define KEYBOARD_EXTENDED_SCAN_CODE 0xE0

static bool shift_down = false;

volatile char last_pressed_char = 0;

static const char scancode_to_ascii[] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' ', 0
};

static const char scancode_to_ascii_shift[] = {
    0, 0, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
    '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0, 'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '\"', '~',
    0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?', 0,
    '*', 0, ' ', 0
};
// -------------------------------

void (*keyboard_handler_user)(struct KeyboardEvent event);

void keyboard_handler() {
    static bool is_extended = 0;
    
    uint8_t scan_code = ps2_read_scan_code();

    __asm__ volatile (
        "outb %%al, %%dx" 
        : 
        : "a"((uint8_t)0x20), "d"((uint16_t)0x20)
    );
    
    if (scan_code == KEYBOARD_EXTENDED_SCAN_CODE) {
        is_extended = true;
        return;
    }
    
    uint16_t fat_code = scan_code & 0x7F;
    
    if (is_extended) {
        is_extended = false;
        fat_code |= KEYBOARD_EXTENDED_SCAN_CODE << 8;
    }
    
    struct KeyboardEvent event;
    
    if ((scan_code & 0x80) == 0) {  // Make code
        event.type = KEYBOARD_EVENT_TYPE_MAKE;
    } else {
        event.type = KEYBOARD_EVENT_TYPE_BREAK;
    }
    
    event.code = fat_code;

    if (fat_code == 0x2A /* Left Shift */ || fat_code == 0x36 /* Right Shift */) {
        if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
            shift_down = true;
        } else if (event.type == KEYBOARD_EVENT_TYPE_BREAK) {
            shift_down = false;
        }
        
        if (keyboard_handler_user != NULL) {
            keyboard_handler_user(event);
        }
        return;
    }


    if (event.type == KEYBOARD_EVENT_TYPE_MAKE && fat_code < sizeof(scancode_to_ascii)) {
        char ascii_char = shift_down ? scancode_to_ascii_shift[fat_code] : scancode_to_ascii[fat_code];
        
        if (ascii_char != 0) {
            last_pressed_char = ascii_char;
        }
    }

    if (keyboard_handler_user != NULL) {
        keyboard_handler_user(event);
    }
}

// Called by Syscall #8
char keyboard_get_last_char() {
    char c = last_pressed_char;
    last_pressed_char = 0; 
    return c;
}

bool keyboard_is_shift_down() {
    return shift_down;
}

void keyboard_init() {
    idt_init();
    idt_set_handler_keyboard(keyboard_handler);
}

void keyboard_set_handler(void (*handler)(struct KeyboardEvent event)) {
    keyboard_handler_user = handler;    
}