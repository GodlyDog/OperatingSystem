#pragma once

#include <drivers/screen.h>
#include <mm/page.h>
#include <types.h>
#include <util/list.h>
//
//
//#define VGA_WIDTH ((uint16_t) 80)
//#define VGA_HEIGHT ((uint16_t) 25)
//#define VGA_AREA ((uint16_t) (VGA_WIDTH * VGA_HEIGHT))
//#define VGA_BUFFER_COUNT ((uint16_t) (1024 * 16))
//#define VGA_BUFFER_SIZE ((uint16_t) (VGA_BUFFER_COUNT * sizeof(short)))
//
//
//#define SCREEN_GET_FOREGROUND(x) ((uint8_t) (x & 0b00001111))
//#define SCREEN_GET_BACKGROUND(x) ((uint8_t) (x & 0b01110000))
//#define SCREEN_MAKE_COLOR(b, f)  ((uint8_t) (b << 4) | f)
//
//#define SCREEN_DEFAULT_FOREGROUND ((uint8_t) 0xF)
//#define SCREEN_DEFAULT_BACKGROUND ((uint8_t) 0x0)
//#define SCREEN_DEFAULT_COLOR SCREEN_MAKE_COLOR(SCREEN_DEFAULT_BACKGROUND,
//SCREEN_DEFAULT_FOREGROUND)

// typedef struct screen {
//    uint16_t screen_cursor_pos;
//    uint16_t screen_buffer_pos;
//    uint16_t screen_visible_pos;
//    uint8_t screen_current_color;
//
//    uint16_t *screen_buffer;
//    uint16_t screen_inactive_buffer[VGA_BUFFER_COUNT];
//} screen_t;

// typedef struct vterminal_char {
//    char c;
////    color_t foreground;
////    color_t background;
//} vterminal_char_t;

#ifdef __VGABUF___

#define VT_PAGES_PER_HISTORY_CHUNK 1
#define VT_CHARS_PER_HISTORY_CHUNK \
    (VT_PAGES_PER_HISTORY_CHUNK * PAGE_SIZE - sizeof(list_link_t))

typedef struct vterminal_history_chunk
{
    char chars[VT_CHARS_PER_HISTORY_CHUNK];
    list_link_t link;
} vterminal_history_chunk_t;

typedef struct vterminal
{
    size_t vt_width;
    size_t vt_height;

    size_t vt_len;
    list_t vt_history_chunks;

    size_t *vt_line_positions;

    off_t vt_line_offset;

    size_t *vt_line_widths;

    size_t vt_input_pos;
    size_t vt_cursor_pos;
} vterminal_t;

void vterminal_init(vterminal_t *vt);

void vterminal_make_active(vterminal_t *vt);

void vterminal_scroll(vterminal_t *vt, long count);

void vterminal_scroll_to_bottom(vterminal_t *t);

void vterminal_clear(vterminal_t *vt);

size_t vterminal_write(vterminal_t *vt, const char *buf, size_t len);

void vterminal_key_pressed(vterminal_t *vt);

#elif 0

struct vt_cursor
{
    int y;
    int x;
};

struct vt_attributes
{
    int underline : 1;
    int bold : 1;
    int blink : 1;
    uint16_t fg;
    uint16_t bg;
};

struct vt_char
{
    int codepoint;
    struct vt_attributes attribs;
};

struct vt_buffer
{
    struct vt_char screen[VGA_HEIGHT][VGA_WIDTH];
    size_t input_position;
};

typedef struct vterminal
{
    size_t height;
    size_t width;
    struct vt_cursor cursor;
    struct vt_cursor saved_cursor;
    struct vt_attributes current_attribs;
    struct vt_buffer *active_buffer;
    struct vt_buffer pri_buffer;
    struct vt_buffer alt_buffer;
} vterminal_t;

void vterminal_init(vterminal_t *vt);

void vterminal_make_active(vterminal_t *vt);

void vterminal_scroll(vterminal_t *vt, long count);

void vterminal_clear(vterminal_t *vt);

size_t vterminal_write(vterminal_t *vt, const char *buf, size_t len);

size_t vterminal_echo_input(vterminal_t *vt, const char *buf, size_t len);

void vterminal_key_pressed(vterminal_t *vt);

void vterminal_scroll_to_bottom(vterminal_t *vt);

#endif

#define VTC_DEFAULT_FOREGROUND VTCOLOR_GREY
#define VTC_DEFAULT_BACKGROUND VTCOLOR_BLACK
#define VTC_DEFAULT_ATTR \
    (vtattr_t) { 0, VTC_DEFAULT_FOREGROUND, VTC_DEFAULT_BACKGROUND }
#define VTC_ANSI_PARSER_STACK_SIZE 8

struct vtconsole;

typedef enum
{
    VTCOLOR_BLACK,
    VTCOLOR_RED,
    VTCOLOR_GREEN,
    VTCOLOR_YELLOW,
    VTCOLOR_BLUE,
    VTCOLOR_MAGENTA,
    VTCOLOR_CYAN,
    VTCOLOR_GREY,
} vtcolor_t;

typedef enum
{
    VTSTATE_ESC,
    VTSTATE_BRACKET,
    VTSTATE_ATTR,
    VTSTATE_ENDVAL,
} vtansi_parser_state_t;

typedef struct
{
    int value;
    int empty;
} vtansi_arg_t;

typedef struct
{
    vtansi_parser_state_t state;
    vtansi_arg_t stack[VTC_ANSI_PARSER_STACK_SIZE];
    int index;
} vtansi_parser_t;

typedef struct
{
    int bright;
    vtcolor_t fg;
    vtcolor_t bg;
} vtattr_t;

typedef struct
{
    char c;
    vtattr_t attr;
} vtcell_t;

typedef struct
{
    int x;
    int y;
} vtcursor_t;

typedef void (*vtc_paint_handler_t)(struct vtconsole *vtc, vtcell_t *cell,
                                    int x, int y);
typedef void (*vtc_cursor_handler_t)(struct vtconsole *vtc, vtcursor_t *cur);

typedef struct vtconsole
{
    int width;
    int height;

    vtattr_t attr;
    vtansi_parser_t ansiparser;

    vtcell_t *buffer;
    int *tabs;
    int tab_index;
    vtcursor_t cursor;

    vtc_paint_handler_t on_paint;
    vtc_cursor_handler_t on_move;
} vtconsole_t;

typedef vtconsole_t vterminal_t;

vtconsole_t *vtconsole(vtconsole_t *vtc, int width, int height,
                       vtc_paint_handler_t on_paint,
                       vtc_cursor_handler_t on_move);
void vtconsole_delete(vtconsole_t *c);

void vtconsole_clear(vtconsole_t *vtc, int fromx, int fromy, int tox, int toy);
void vtconsole_scroll(vtconsole_t *vtc, int lines);
void vtconsole_newline(vtconsole_t *vtc);

void vtconsole_putchar(vtconsole_t *vtc, char c);
void vtconsole_write(vtconsole_t *vtc, const char *buffer, uint32_t size);

size_t vterminal_write(vterminal_t *vt, const char *buf, size_t len);

size_t vterminal_echo_input(vterminal_t *vt, const char *buf, size_t len);

void vterminal_key_pressed(vterminal_t *vt);

void vterminal_scroll_to_bottom(vterminal_t *vt);

void vterminal_init(vterminal_t *vt);

void vterminal_make_active(vterminal_t *vt);
