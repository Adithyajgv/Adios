#pragma once

#include <stdint.h>
#include "bool.h"

enum {
	KEYBOARD_EVENT_TYPE_MAKE = 0,
	KEYBOARD_EVENT_TYPE_BREAK = 1,
};

struct KeyboardEvent {
	uint8_t type;
	uint16_t code;
};

void keyboard_init();
void keyboard_set_handler(void (*handler)(struct KeyboardEvent event));
bool keyboard_is_shift_down();
