#include "print.h"
#include "keyboard.h"
#include "x86_64/rtc.h"
#include "x86_64/terminal.h"
#include "x86_64/keycodes.h"

void handle_input(struct KeyboardEvent event) {
    if (event.type == KEYBOARD_EVENT_TYPE_MAKE) {
        print_set_color(PRINT_COLOR_BLUE, PRINT_COLOR_WHITE);
        print_char(to_ascii(event.code, false));
    } else if (event.type == KEYBOARD_EVENT_TYPE_BREAK) {
    }
}

void kernel_main() {
    print_clear();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print_str("Welcome to AdiOS!");
    
    keyboard_init();    
    terminal_init();
    
    while (1){
        terminal_run();
    }
}
