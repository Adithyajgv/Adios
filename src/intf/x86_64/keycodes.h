#pragma once
#include <stdint.h>

#define KEY_CODE_A 0x1E
#define KEY_CODE_B 0x30
#define KEY_CODE_C 0x2E
#define KEY_CODE_D 0x20
#define KEY_CODE_E 0x12
#define KEY_CODE_F 0x21
#define KEY_CODE_G 0x22
#define KEY_CODE_H 0x23
#define KEY_CODE_I 0x17
#define KEY_CODE_J 0x24
#define KEY_CODE_K 0x25
#define KEY_CODE_L 0x26
#define KEY_CODE_M 0x32
#define KEY_CODE_N 0x31
#define KEY_CODE_O 0x18
#define KEY_CODE_P 0x19
#define KEY_CODE_Q 0x10
#define KEY_CODE_R 0x13
#define KEY_CODE_S 0x1F
#define KEY_CODE_T 0x14
#define KEY_CODE_U 0x16
#define KEY_CODE_V 0x2F
#define KEY_CODE_W 0x11
#define KEY_CODE_X 0x2D
#define KEY_CODE_Y 0x15
#define KEY_CODE_Z 0x2C
#define KEY_CODE_SPACE 0x39
#define KEY_CODE_ENTER 0x1C
#define KEY_CODE_DOT      0x34  // '.'
#define KEY_CODE_COMMA    0x33  // ','
#define KEY_CODE_SLASH    0x35  // '/'
#define KEY_CODE_SEMICOLON 0x27 // ';'
#define KEY_CODE_QUOTE     0x28 // '''
#define KEY_CODE_MINUS     0x0C // '-'
#define KEY_CODE_EQUAL     0x0D // '='
#define KEY_CODE_BACKSLASH 0x2B // '\'
#define KEY_CODE_LEFTBRACKET  0x1A // '['
#define KEY_CODE_RIGHTBRACKET 0x1B // ']'
#define KEY_CODE_BACKSPACE 0x0E

char to_ascii(uint16_t code);