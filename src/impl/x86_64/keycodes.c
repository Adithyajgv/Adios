#include "x86_64/keycodes.h"

char to_ascii(uint16_t code, bool shift) {
    switch (code) {
        // Letters
        case KEY_CODE_A: return shift ? 'A' : 'a';
        case KEY_CODE_B: return shift ? 'B' : 'b';
        case KEY_CODE_C: return shift ? 'C' : 'c';
        case KEY_CODE_D: return shift ? 'D' : 'd';
        case KEY_CODE_E: return shift ? 'E' : 'e';
        case KEY_CODE_F: return shift ? 'F' : 'f';
        case KEY_CODE_G: return shift ? 'G' : 'g';
        case KEY_CODE_H: return shift ? 'H' : 'h';
        case KEY_CODE_I: return shift ? 'I' : 'i';
        case KEY_CODE_J: return shift ? 'J' : 'j';
        case KEY_CODE_K: return shift ? 'K' : 'k';
        case KEY_CODE_L: return shift ? 'L' : 'l';
        case KEY_CODE_M: return shift ? 'M' : 'm';
        case KEY_CODE_N: return shift ? 'N' : 'n';
        case KEY_CODE_O: return shift ? 'O' : 'o';
        case KEY_CODE_P: return shift ? 'P' : 'p';
        case KEY_CODE_Q: return shift ? 'Q' : 'q';
        case KEY_CODE_R: return shift ? 'R' : 'r';
        case KEY_CODE_S: return shift ? 'S' : 's';
        case KEY_CODE_T: return shift ? 'T' : 't';
        case KEY_CODE_U: return shift ? 'U' : 'u';
        case KEY_CODE_V: return shift ? 'V' : 'v';
        case KEY_CODE_W: return shift ? 'W' : 'w';
        case KEY_CODE_X: return shift ? 'X' : 'x';
        case KEY_CODE_Y: return shift ? 'Y' : 'y';
        case KEY_CODE_Z: return shift ? 'Z' : 'z';

        // Digits and corresponding shifted symbols
        case 0x02: return shift ? '!' : '1'; // '1' keycode
        case 0x03: return shift ? '@' : '2';
        case 0x04: return shift ? '#' : '3';
        case 0x05: return shift ? '$' : '4';
        case 0x06: return shift ? '%' : '5';
        case 0x07: return shift ? '^' : '6';
        case 0x08: return shift ? '&' : '7';
        case 0x09: return shift ? '*' : '8';
        case 0x0A: return shift ? '(' : '9';
        case 0x0B: return shift ? ')' : '0';

        // Punctuation and symbols
        case KEY_CODE_MINUS: return shift ? '_' : '-';
        case KEY_CODE_EQUAL: return shift ? '+' : '=';
        case KEY_CODE_LEFTBRACKET: return shift ? '{' : '[';
        case KEY_CODE_RIGHTBRACKET: return shift ? '}' : ']';
        case KEY_CODE_BACKSLASH: return shift ? '|' : '\\';
        case KEY_CODE_SEMICOLON: return shift ? ':' : ';';
        case KEY_CODE_QUOTE: return shift ? '"' : '\'';
        case KEY_CODE_COMMA: return shift ? '<' : ',';
        case KEY_CODE_DOT: return shift ? '>' : '.';
        case KEY_CODE_SLASH: return shift ? '?' : '/';

        // Space and Enter
        case KEY_CODE_SPACE: return ' ';
        case KEY_CODE_ENTER: return '\n';

        default: return '?';
    }
}
