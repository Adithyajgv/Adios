#include "x86_64/keycodes.h"

char to_ascii(uint16_t code) {
    switch (code) {
        case KEY_CODE_A: return 'a';
        case KEY_CODE_B: return 'b';
        case KEY_CODE_C: return 'c';
        case KEY_CODE_D: return 'd';
        case KEY_CODE_E: return 'e';
        case KEY_CODE_F: return 'f';
        case KEY_CODE_G: return 'g';
        case KEY_CODE_H: return 'h';
        case KEY_CODE_I: return 'i';
        case KEY_CODE_J: return 'j';
        case KEY_CODE_K: return 'k';
        case KEY_CODE_L: return 'l';
        case KEY_CODE_M: return 'm';
        case KEY_CODE_N: return 'n';
        case KEY_CODE_O: return 'o';
        case KEY_CODE_P: return 'p';
        case KEY_CODE_Q: return 'q';
        case KEY_CODE_R: return 'r';
        case KEY_CODE_S: return 's';
        case KEY_CODE_T: return 't';
        case KEY_CODE_U: return 'u';
        case KEY_CODE_V: return 'v';
        case KEY_CODE_W: return 'w';
        case KEY_CODE_X: return 'x';
        case KEY_CODE_Y: return 'y';
        case KEY_CODE_Z: return 'z';
        case KEY_CODE_SPACE: return ' ';
        case KEY_CODE_ENTER: return '\n';
        case KEY_CODE_DOT: return '.';
        case KEY_CODE_COMMA: return ',';
        case KEY_CODE_SLASH: return '/';
        case KEY_CODE_SEMICOLON: return ';';
        case KEY_CODE_QUOTE: return '\'';
        case KEY_CODE_MINUS: return '-';
        case KEY_CODE_EQUAL: return '=';
        case KEY_CODE_BACKSLASH: return '\\';
        case KEY_CODE_LEFTBRACKET: return '[';
        case KEY_CODE_RIGHTBRACKET: return ']';
    }    
    
    return '?';
}