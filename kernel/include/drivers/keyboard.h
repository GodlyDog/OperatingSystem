#pragma once

#include <types.h>

#define BS 0x08
#define DEL 0x7F
#define ESC 0x1B
#define LF 0x0A
#define CR 0x0D
#define SPACE 0x20

// CTRL-D
#define EOT 0x04

// CTRL-C
#define ETX 0x03

/* Special stuff for scrolling (note that these only work when ctrl is held) */
#define SCROLL_UP 0x0e
#define SCROLL_DOWN 0x1c
#define SCROLL_UP_PAGE 0x0f
#define SCROLL_DOWN_PAGE 0x1d

// pretty arbitrarily chosen, just the first extended ASCII code point and on...
#define F1 ((uint8_t)128)
#define F2 ((uint8_t)(F1 + 1))
#define F3 ((uint8_t)(F1 + 2))
#define F4 ((uint8_t)(F1 + 3))
#define F5 ((uint8_t)(F1 + 4))
#define F6 ((uint8_t)(F1 + 5))
#define F7 ((uint8_t)(F1 + 6))
#define F8 ((uint8_t)(F1 + 7))
#define F9 ((uint8_t)(F1 + 8))
#define F10 ((uint8_t)(F1 + 9))
#define F11 ((uint8_t)(F1 + 10))
#define F12 ((uint8_t)(F1 + 11))

typedef void (*keyboard_char_handler_t)(uint8_t);

/**
 * Initializes the keyboard subsystem.
 */
void keyboard_init(keyboard_char_handler_t handler);
