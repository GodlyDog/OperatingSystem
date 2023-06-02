#pragma once

#include "types.h"

#ifdef __VGABUF___

#define SCREEN_CHARACTER_WIDTH 9
#define SCREEN_CHARACTER_HEIGHT 15

typedef union color {
    struct
    {
        uint8_t blue;
        uint8_t green;
        uint8_t red;
        uint8_t alpha;
    } channels;
    uint32_t value;
} packed color_t;

void screen_init();

size_t screen_get_width();

size_t screen_get_height();

size_t screen_get_character_width();

size_t screen_get_character_height();

void screen_draw_string(size_t x, size_t y, const char *s, size_t len,
                        color_t color);

void screen_fill(color_t color);

void screen_fill_rect(size_t x, size_t y, size_t width, size_t height,
                      color_t color);

void screen_draw_rect(size_t x, size_t y, size_t width, size_t height,
                      color_t color);

void screen_copy_rect(size_t fromx, size_t fromy, size_t width, size_t height,
                      size_t tox, size_t toy);

void screen_flush();

void screen_print_shutdown();

#else

#define VGA_WIDTH ((uint16_t)80)
#define VGA_HEIGHT ((uint16_t)25)
#define VGA_LINE_SIZE ((size_t)(VGA_WIDTH * sizeof(uint16_t)))
#define VGA_AREA ((uint16_t)(VGA_WIDTH * VGA_HEIGHT))
#define VGA_BUFFER_SIZE ((uint16_t)(VGA_WIDTH * VGA_HEIGHT))
#define VGA_DEFAULT_ATTRIB 0xF

void vga_init();

void vga_write_char_at(size_t row, size_t col, uint16_t v);

void vga_set_cursor(size_t row, size_t col);

void vga_clear_screen();

void screen_print_shutdown();

void vga_enable_cursor();

void vga_disable_cursor();

#endif