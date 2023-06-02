#include <drivers/keyboard.h>
#include <drivers/tty/ldisc.h>
#include <drivers/tty/tty.h>
#include <drivers/tty/vterminal.h>
#include <errno.h>
#include <mm/kmalloc.h>
#include <util/debug.h>
#include <util/string.h>

/*

vterminal.c is used to manage the display of the terminal screen, this includes
printing the keys pressed, output of the command passed, managing the cursor 
position, etc. 

vterminal_write is called by functions in tty.c and ldisc.c, namely tty_write
and ldisc_key_pressed. vterminal_write then calls vtconsole_write which takes 
care of the processing of the characters with the help of vtconsole_process
vtconsole_process and vtconsole_append are responsible for printing the
characters corresponding to the keys pressed onto the console. 

vtconsole_append also manages the position of the cursor while the uncooked 
part of the buffer is being printed. There are mutltiple other functions defined
in this file which help in displaying the cursor on the console. The console
also supports scrolling which is handled by vtconsole_scroll. vterminal_clear
is used to clear the content of the console.

The functions, vterminal_make_active, vterminal_init, vtconsole, paint_callback
and cursor_move_callback are responsible for carrying out the necessary
initialization and initial display of the console.

*/

#define vterminal_to_tty(vterminal) \
    CONTAINER_OF((vterminal), tty_t, tty_vterminal)

#ifdef __VGABUF___

/*
Without turning on VGABUF, the terminal is treated as a simple device: one sent characters 
to it to be displayed. It did the right thing with new lines and with backspaces, 
but didn't handle any other control characters. The VGA handles all sorts of other things, 
but we also have to explicitly tell it to scroll. VGABUF allows Weenix to toggle between 
VGA text mode (that understands text) and VGA buffer mode (that is pixel based).
*/

#define VT_LINE_POSITION(vt, line)                                     \
    ((vt)->vt_line_positions[((vt)->vt_line_offset + (vt)->vt_height + \
                              (line)) %                                \
                             (vt)->vt_height])

#define vterminal_to_tty(vterminal) \
    CONTAINER_OF((vterminal), tty_t, tty_vterminal)

#define VT_OFFSCREEN ((size_t)-1)

static long vterminal_add_chunk(vterminal_t *vt);

static vterminal_t *active_vt = NULL;

void vterminal_init(vterminal_t *vt)
{
    vt->vt_width = screen_get_width() / screen_get_character_width();
    vt->vt_height = screen_get_height() / screen_get_character_height();
    list_init(&vt->vt_history_chunks);
    vt->vt_line_positions = kmalloc(sizeof(size_t) * vt->vt_height * 2);
    KASSERT(vt->vt_line_positions);
    vt->vt_line_widths = vt->vt_line_positions + vt->vt_height;

    list_init(&vt->vt_history_chunks);
    long success = vterminal_add_chunk(vt);
    KASSERT(success && !list_empty(&vt->vt_history_chunks));

    vterminal_clear(vt);
}

static void vterminal_seek_to_pos(vterminal_t *vt, size_t pos,
                                  vterminal_history_chunk_t **chunk,
                                  size_t *offset)
{
    if (pos > vt->vt_len)
    {
        *chunk = NULL;
        *offset = 0;
        return;
    }
    *offset = pos % VT_CHARS_PER_HISTORY_CHUNK;
    size_t n_chunks = vt->vt_len / VT_CHARS_PER_HISTORY_CHUNK;
    size_t iterations = pos / VT_CHARS_PER_HISTORY_CHUNK;
    if (iterations > n_chunks >> 1)
    {
        iterations = n_chunks - iterations;
        list_iterate_reverse(&vt->vt_history_chunks, chunk_iter,
                             vterminal_history_chunk_t, link)
        {
            if (!iterations--)
            {
                *chunk = chunk_iter;
                return;
            }
        }
    }
    else
    {
        list_iterate(&vt->vt_history_chunks, chunk_iter,
                     vterminal_history_chunk_t, link)
        {
            if (!iterations--)
            {
                *chunk = chunk_iter;
                return;
            }
        }
    }
}

static inline long vterminal_seek_to_offset(vterminal_t *vt,
                                            vterminal_history_chunk_t **chunk,
                                            size_t *offset)
{
    while (*offset >= VT_CHARS_PER_HISTORY_CHUNK)
    {
        if (*chunk ==
            list_tail(&vt->vt_history_chunks, vterminal_history_chunk_t, link))
            return 0;
        *chunk = list_next(*chunk, vterminal_history_chunk_t, link);
        *offset -= VT_CHARS_PER_HISTORY_CHUNK;
    }
    return 1;
}

size_t vterminal_calculate_line_width_forward(vterminal_t *vt, size_t pos)
{
    vterminal_history_chunk_t *chunk;
    size_t offset;
    vterminal_seek_to_pos(vt, pos, &chunk, &offset);
    if (!chunk)
        return 0;
    size_t width = 0;
    while (pos + width < vt->vt_len && chunk->chars[offset++] != LF)
    {
        width++;
        if (!vterminal_seek_to_offset(vt, &chunk, &offset))
            break;
    }
    return width;
}
static void vterminal_redraw_lines(vterminal_t *vt, size_t start, size_t end)
{
    KASSERT(start < vt->vt_height && start < end && end <= vt->vt_height);

    size_t pos = VT_LINE_POSITION(vt, start);
    vterminal_history_chunk_t *chunk;
    size_t offset;
    vterminal_seek_to_pos(vt, pos, &chunk, &offset);

    color_t cursor = {.value = 0x00D3D3D3};
    color_t background = {.value = 0x00000000};
    color_t foreground = {.value = 0x00FFFFFF};

    size_t screen_y = screen_get_character_height() * start;

    size_t line = start;
    while (line < end && pos <= vt->vt_len &&
           vterminal_seek_to_offset(vt, &chunk, &offset))
    {
        KASSERT(pos == VT_LINE_POSITION(vt, line));

        size_t cur_width = vt->vt_line_widths[line];
        size_t new_width, next_pos;
        if (line + 1 < vt->vt_height &&
            (next_pos = VT_LINE_POSITION(vt, line + 1)) != VT_OFFSCREEN)
        {
            new_width = next_pos - pos - 1;
        }
        else
        {
            new_width = vterminal_calculate_line_width_forward(vt, pos);
        }
        vt->vt_line_widths[line] = new_width;

        screen_fill_rect(
            0, screen_y,
            MAX(cur_width, new_width) * screen_get_character_width(),
            screen_get_character_height(), background);
        if (pos <= vt->vt_cursor_pos && vt->vt_cursor_pos <= pos + new_width)
        {
            screen_fill_rect(
                (vt->vt_cursor_pos - pos) * screen_get_character_width(),
                screen_y, screen_get_character_width(),
                screen_get_character_height(), cursor);
            vt->vt_line_widths[line]++;
        }
        size_t drawn = 0;
        while (drawn != new_width)
        {
            size_t to_draw =
                MIN(VT_CHARS_PER_HISTORY_CHUNK - offset, new_width - drawn);
            screen_draw_string(drawn * screen_get_character_width(), screen_y,
                               chunk->chars + offset, to_draw, foreground);
            drawn += to_draw;
            offset += to_draw;
            if (!vterminal_seek_to_offset(vt, &chunk, &offset))
            {
                vterminal_seek_to_offset(vt, &chunk, &offset);
                KASSERT(drawn == new_width);
                break;
            }
        }

        pos += new_width + 1;
        KASSERT(chunk->chars[offset] == LF || pos >= vt->vt_len);

        offset++;
        line++;
        screen_y += screen_get_character_height();
    }
    while (line < end)
    {
        //        dbg(DBG_TEMP, "clearing line %lu\n", line);
        screen_fill_rect(
            0, screen_y,
            vt->vt_line_widths[line] * screen_get_character_width(),
            screen_get_character_height(), background);
        vt->vt_line_widths[line] = 0;
        line++;
        screen_y += screen_get_character_height();
    }
}

void vterminal_make_active(vterminal_t *vt)
{
    KASSERT(vt);
    if (active_vt == vt)
        return;
    active_vt = vt;
    for (size_t line = 0; line < vt->vt_height; line++)
    {
        vt->vt_line_widths[line] = vt->vt_width;
    }
    color_t background = {.value = 0x00000000};
    screen_fill_rect(
        vt->vt_width * screen_get_character_width(), 0,
        screen_get_width() - vt->vt_width * screen_get_character_width(),
        screen_get_height(), background);
    screen_fill_rect(
        0, vt->vt_height * screen_get_character_height(), screen_get_width(),
        screen_get_height() - vt->vt_height * screen_get_character_height(),
        background);
    vterminal_redraw_lines(vt, 0, vt->vt_height);
}

size_t vterminal_calculate_line_width_backward(vterminal_t *vt, size_t pos)
{
    if (!pos)
        return 0;
    vterminal_history_chunk_t *chunk;
    size_t offset;
    vterminal_seek_to_pos(vt, pos - 1, &chunk, &offset);
    size_t width = 0;
    while (chunk->chars[offset] != LF)
    {
        width++;
        if (offset == 0)
        {
            if (chunk == list_head(&vt->vt_history_chunks,
                                   vterminal_history_chunk_t, link))
                break;
            chunk = list_prev(chunk, vterminal_history_chunk_t, link);
            offset = VT_CHARS_PER_HISTORY_CHUNK;
        }
        offset--;
    }
    return width;
}

static inline void vterminal_get_last_visible_line_information(vterminal_t *vt,
                                                               size_t *position,
                                                               size_t *width)
{
    for (long line = vt->vt_height - 1; line >= 0; line--)
    {
        if (VT_LINE_POSITION(vt, line) != VT_OFFSCREEN)
        {
            *position = VT_LINE_POSITION(vt, line);
            *width = vterminal_calculate_line_width_forward(vt, *position);
            return;
        }
    }
    panic("should always find last visible line information");
}

static inline long vterminal_scrolled_to_bottom(vterminal_t *vt)
{
    size_t position;
    size_t width;
    vterminal_get_last_visible_line_information(vt, &position, &width);
    return position + width == vt->vt_len;
}

void vterminal_scroll_to_bottom(vterminal_t *vt)
{
    if (vterminal_scrolled_to_bottom(vt))
        return;
    vt->vt_line_offset = 0;
    VT_LINE_POSITION(vt, 0) = vt->vt_len + 1;
    vterminal_scroll(vt, -vt->vt_height);
    for (size_t line = vt->vt_height - vt->vt_line_offset; line < vt->vt_height;
         line++)
    {
        VT_LINE_POSITION(vt, line) = VT_OFFSCREEN;
    }
}

void vterminal_scroll_draw(vterminal_t *vt, long count)
{
    if (count > 0)
    {
        if ((size_t)count > vt->vt_height)
            count = vt->vt_height;
        size_t copy_distance = count * screen_get_character_height();
        size_t screen_y = 0;
        for (size_t line = 0; line < vt->vt_height - count; line++)
        {
            screen_copy_rect(0, screen_y + copy_distance,
                             MAX(vt->vt_line_widths[line],
                                 vt->vt_line_widths[line + count]) *
                                 screen_get_character_width(),
                             screen_get_character_height(), 0, screen_y);
            vt->vt_line_widths[line] = vt->vt_line_widths[line + count];
            screen_y += screen_get_character_height();
        }
        vterminal_redraw_lines(vt, vt->vt_height - count, vt->vt_height);
    }
    else if (count < 0)
    {
        count *= -1;
        if ((size_t)count > vt->vt_height)
            count = vt->vt_height;
        size_t copy_distance = count * screen_get_character_height();
        size_t screen_y =
            (vt->vt_height - count) * screen_get_character_height();
        for (size_t line = vt->vt_height - count; line >= (size_t)count;
             line--)
        {
            screen_copy_rect(0, screen_y - copy_distance,
                             MAX(vt->vt_line_widths[line],
                                 vt->vt_line_widths[line - count]) *
                                 screen_get_character_width(),
                             screen_get_character_height(), 0, screen_y);
            vt->vt_line_widths[line] = vt->vt_line_widths[line - count];
            screen_y -= screen_get_character_height();
        }
        vterminal_redraw_lines(vt, 0, (size_t)count);
    }
}

void vterminal_scroll(vterminal_t *vt, long count)
{
    long n_scrolls = 0;
    if (count < 0)
    {
        size_t first_line_position = VT_LINE_POSITION(vt, 0);
        while (count++ && first_line_position)
        {
            size_t width = vterminal_calculate_line_width_backward(
                vt, first_line_position - 1);
            size_t top_line_position = first_line_position - width - 1;
            VT_LINE_POSITION(vt, -1) = top_line_position;
            if (!vt->vt_line_offset)
                vt->vt_line_offset = vt->vt_height;
            vt->vt_line_offset--;
            n_scrolls++;
            first_line_position = top_line_position;
        }
        if (n_scrolls)
        {
            vterminal_scroll_draw(vt, -n_scrolls);
        }
    }
    else if (count > 0)
    {
        size_t last_line_position;
        size_t last_line_width;
        vterminal_get_last_visible_line_information(vt, &last_line_position,
                                                    &last_line_width);
        while (count-- && last_line_position + last_line_width < vt->vt_len)
        {
            size_t bottom_line_position =
                last_line_position + last_line_width + 1;
            VT_LINE_POSITION(vt, 0) = bottom_line_position;
            vt->vt_line_offset++;
            if ((unsigned)vt->vt_line_offset == vt->vt_height)
                vt->vt_line_offset = 0;
            n_scrolls++;
            last_line_position = bottom_line_position;
            last_line_width =
                vterminal_calculate_line_width_forward(vt, last_line_position);
        }
        if (n_scrolls)
        {
            vterminal_scroll_draw(vt, n_scrolls);
        }
    }
}

void vterminal_clear(vterminal_t *vt)
{
    list_iterate(&vt->vt_history_chunks, chunk, vterminal_history_chunk_t,
                 link)
    {
        if (chunk != list_tail(&vt->vt_history_chunks,
                               vterminal_history_chunk_t, link))
        {
            list_remove(&chunk->link);
            page_free_n(chunk, VT_PAGES_PER_HISTORY_CHUNK);
        }
        else
        {
            memset(chunk, 0, VT_CHARS_PER_HISTORY_CHUNK);
        }
    }
    vt->vt_len = 0;
    for (size_t i = 0; i < vt->vt_height; i++)
    {
        vt->vt_line_widths[i] = 0;
        vt->vt_line_positions[i] = VT_OFFSCREEN;
    }
    vt->vt_line_offset = 0;
    vt->vt_cursor_pos = 0;
    vt->vt_input_pos = 0;
    VT_LINE_POSITION(vt, 0) = 0;
}

static long vterminal_add_chunk(vterminal_t *vt)
{
    vterminal_history_chunk_t *chunk = page_alloc_n(VT_PAGES_PER_HISTORY_CHUNK);
    if (!chunk)
    {
        chunk =
            list_head(&vt->vt_history_chunks, vterminal_history_chunk_t, link);
        if (chunk ==
            list_tail(&vt->vt_history_chunks, vterminal_history_chunk_t, link))
            return 0;
        list_remove(&chunk->link);

        // TODO what if the first chunk that we're removing is visible? lol
        for (size_t i = 0; i < vt->vt_height; i++)
        {
            KASSERT(vt->vt_line_positions[i] >= VT_CHARS_PER_HISTORY_CHUNK &&
                    "NYI");
            vt->vt_line_positions[i] -= VT_CHARS_PER_HISTORY_CHUNK;
        }
        KASSERT(vt->vt_input_pos >= VT_CHARS_PER_HISTORY_CHUNK &&
                vt->vt_cursor_pos >= VT_CHARS_PER_HISTORY_CHUNK &&
                vt->vt_len >= VT_CHARS_PER_HISTORY_CHUNK && "NYI");
        vt->vt_input_pos -= VT_CHARS_PER_HISTORY_CHUNK;
        vt->vt_cursor_pos -= VT_CHARS_PER_HISTORY_CHUNK;
        vt->vt_len -= VT_CHARS_PER_HISTORY_CHUNK;
    }

    memset(chunk, 0, sizeof(vterminal_history_chunk_t));

    list_link_init(&chunk->link);
    list_insert_tail(&vt->vt_history_chunks, &chunk->link);

    return 1;
}

static inline long vterminal_allocate_to_offset(
    vterminal_t *vt, vterminal_history_chunk_t **chunk, size_t *offset)
{
    if (!vterminal_seek_to_offset(vt, chunk, offset))
    {
        if (!vterminal_add_chunk(vt))
        {
            return 0;
        }
        return vterminal_seek_to_offset(vt, chunk, offset);
    }
    return 1;
}

size_t vterminal_write(vterminal_t *vt, const char *buf, size_t len)
{
    size_t written = 0;

    size_t last_line_width =
        vterminal_calculate_line_width_backward(vt, vt->vt_len);
    size_t last_line_idx;
    size_t last_line_position = VT_OFFSCREEN;
    for (last_line_idx = vt->vt_height - 1;; last_line_idx--)
    {
        if ((last_line_position = VT_LINE_POSITION(vt, last_line_idx)) !=
            VT_OFFSCREEN)
        {
            break;
        }
    }
    KASSERT(last_line_idx < vt->vt_height);

    vterminal_history_chunk_t *chunk;
    size_t offset;
    vterminal_seek_to_pos(vt, vt->vt_len, &chunk, &offset);

    size_t last_line_idx_initial = (size_t)last_line_idx;

    long need_to_scroll = last_line_position + last_line_width == vt->vt_len;
    size_t n_scroll_downs = 0;
    while (len--)
    {
        char c = *(buf++);
        written++;
        if (c != LF)
        {
            chunk->chars[offset++] = c;
            vt->vt_len++;
            last_line_width++;
            if (!vterminal_allocate_to_offset(vt, &chunk, &offset))
                goto done;
        }
        if (last_line_width == vt->vt_width)
        {
            c = LF;
        }
        if (c == LF)
        {
            chunk->chars[offset++] = LF;
            vt->vt_len++;
            if (!vterminal_allocate_to_offset(vt, &chunk, &offset))
                goto done;

            if (need_to_scroll)
            {
                KASSERT(last_line_position + last_line_width + 1 == vt->vt_len);
                if (last_line_idx == vt->vt_height - 1)
                {
                    vt->vt_line_offset++;
                    n_scroll_downs++;
                    if ((unsigned)vt->vt_line_offset == vt->vt_height)
                        vt->vt_line_offset = 0;
                    if (last_line_idx_initial)
                        last_line_idx_initial--;
                }
                else
                {
                    last_line_idx++;
                }
                last_line_width = 0;
                last_line_position = VT_LINE_POSITION(vt, last_line_idx) =
                    vt->vt_len;
            }
        }
    }

    last_line_idx++;
done:
    vt->vt_input_pos = vt->vt_len;
    vt->vt_cursor_pos = vt->vt_len;

    if (need_to_scroll)
    {
        if (active_vt == vt)
        {
            if (last_line_idx >= vt->vt_height &&
                n_scroll_downs < vt->vt_height)
            {
                vterminal_scroll_draw(vt, n_scroll_downs);
                last_line_idx = vt->vt_height;
            }
            vterminal_redraw_lines(vt, last_line_idx_initial,
                                   MIN(last_line_idx, vt->vt_height));
        }
        else
        {
            vterminal_scroll(vt, n_scroll_downs);
        }
    }
    return written;
}

static void vterminal_free_from_position_to_end(vterminal_t *vt, size_t pos)
{
    vterminal_history_chunk_t *chunk;
    size_t offset;
    vterminal_seek_to_pos(vt, vt->vt_input_pos, &chunk, &offset);
    while (chunk !=
           list_tail(&vt->vt_history_chunks, vterminal_history_chunk_t, link))
    {
        vterminal_history_chunk_t *to_remove =
            list_tail(&vt->vt_history_chunks, vterminal_history_chunk_t, link);
        list_remove(&to_remove->link);
        page_free_n(to_remove, VT_PAGES_PER_HISTORY_CHUNK);
    }
    vt->vt_len = pos;
    for (size_t line = 0; line < vt->vt_height; line++)
    {
        if (VT_LINE_POSITION(vt, line) > vt->vt_len)
        {
            VT_LINE_POSITION(vt, line) = VT_OFFSCREEN;
            vterminal_redraw_lines(vt, line, line + 1);
        }
    }
}

void vterminal_key_pressed(vterminal_t *vt)
{
    KASSERT(active_vt == vt);
    vterminal_scroll_to_bottom(vt);
    char buf[LDISC_BUFFER_SIZE];
    size_t len =
        ldisc_get_current_line_raw(&vterminal_to_tty(vt)->tty_ldisc, buf);
    size_t initial_input_pos = vt->vt_input_pos;
    vterminal_free_from_position_to_end(vt, initial_input_pos);
    vterminal_write(vt, buf, len);

    vt->vt_input_pos = initial_input_pos;
}

#endif

#define VGA_SCREEN_WIDTH 80
#define VGA_SCREEN_HEIGHT 25

#define VGACOLOR_BLACK 0X0
#define VGACOLOR_BLUE 0X1
#define VGACOLOR_GREEN 0X2
#define VGACOLOR_CYAN 0X3
#define VGACOLOR_RED 0X4
#define VGACOLOR_MAGENTA 0X5
#define VGACOLOR_BROWN 0X6
#define VGACOLOR_LIGHT_GRAY 0X7
#define VGACOLOR_GRAY 0X8
#define VGACOLOR_LIGHT_BLUE 0X9
#define VGACOLOR_LIGHT_GREEN 0XA
#define VGACOLOR_LIGHT_CYAN 0XB
#define VGACOLOR_LIGHT_RED 0XC
#define VGACOLOR_LIGHT_MAGENTA 0XD
#define VGACOLOR_LIGHT_YELLOW 0XE
#define VGACOLOR_WHITE 0XF

/* --- Constructor/Destructor ----------------------------------------------- */

// vtconsole contructor/init function
vtconsole_t *vtconsole(vtconsole_t *vtc, int width, int height,
                       vtc_paint_handler_t on_paint,
                       vtc_cursor_handler_t on_move)
{
    vtc->width = width;
    vtc->height = height;

    vtc->ansiparser = (vtansi_parser_t){VTSTATE_ESC, {{0, 0}}, 0};
    vtc->attr = VTC_DEFAULT_ATTR;

    vtc->buffer = kmalloc(width * height * sizeof(vtcell_t));

    vtc->tabs = kmalloc(LDISC_BUFFER_SIZE * sizeof(int));
    vtc->tab_index = 0;

    vtc->cursor = (vtcursor_t){0, 0};

    vtc->on_paint = on_paint;
    vtc->on_move = on_move;

    vtconsole_clear(vtc, 0, 0, width, height - 1);

    return vtc;
}

// function to free the vtconosle/vterminal buffer
void vtconsole_delete(vtconsole_t *vtc)
{
    kfree(vtc->buffer);
    kfree(vtc->tabs);
    kfree(vtc);
}

/* --- Internal methods ---------------------------------------------------- */

// function to clear everything on the vterminal
void vtconsole_clear(vtconsole_t *vtc, int fromx, int fromy, int tox, int toy)
{
    for (int i = fromx + fromy * vtc->width; i < tox + toy * vtc->width; i++)
    {
        vtcell_t *cell = &vtc->buffer[i];

        cell->attr = VTC_DEFAULT_ATTR;
        cell->c = ' ';

        if (vtc->on_paint)
        {
            vtc->on_paint(vtc, cell, i % vtc->width, i / vtc->width);
        }
    }
}

// helper function for vtconsole_newline to scroll down the screen.
void vtconsole_scroll(vtconsole_t *vtc, int lines)
{
    if (lines == 0)
        return;

    lines = lines > vtc->height ? vtc->height : lines;

    // Scroll the screen by number of $lines.
    for (int i = 0; i < ((vtc->width * vtc->height) - (vtc->width * lines));
         i++)
    {
        vtc->buffer[i] = vtc->buffer[i + (vtc->width * lines)];

        if (vtc->on_paint)
        {
            vtc->on_paint(vtc, &vtc->buffer[i], i % vtc->width, i / vtc->width);
        }
    }

    // Clear the last $lines.
    for (int i = ((vtc->width * vtc->height) - (vtc->width * lines));
         i < vtc->width * vtc->height; i++)
    {
        vtcell_t *cell = &vtc->buffer[i];
        cell->attr = VTC_DEFAULT_ATTR;
        cell->c = ' ';

        if (vtc->on_paint)
        {
            vtc->on_paint(vtc, &vtc->buffer[i], i % vtc->width, i / vtc->width);
        }
    }

    // Move the cursor up $lines
    if (vtc->cursor.y > 0)
    {
        vtc->cursor.y -= lines;

        if (vtc->cursor.y < 0)
            vtc->cursor.y = 0;

        if (vtc->on_move)
        {
            vtc->on_move(vtc, &vtc->cursor);
        }
    }
}

// Append a new line
void vtconsole_newline(vtconsole_t *vtc)
{
    vtc->cursor.x = 0;
    vtc->cursor.y++;

    if (vtc->cursor.y == vtc->height)
    {
        vtconsole_scroll(vtc, 1);
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Append character to the console buffer.
void vtconsole_append(vtconsole_t *vtc, char c)
{
    if (c == '\n')
    {
        vtconsole_newline(vtc);
    }
    else if (c == '\r')
    {
        vtc->cursor.x = 0;

        if (vtc->on_move)
        {
            vtc->on_move(vtc, &vtc->cursor);
        }
    }
    else if (c == '\t')
    {
        int n = 8 - (vtc->cursor.x % 8);
        // storing all the tabs and their size encountered.
        vtc->tabs[vtc->tab_index % LDISC_BUFFER_SIZE] = n;
        vtc->tab_index++;

        for (int i = 0; i < n; i++)
        {
            vtconsole_append(vtc, ' ');
        }
    }
    else if (c == '\b')
    {
        if (vtc->cursor.x > 0)
        {
            vtc->cursor.x--;
        }
        else
        {
            vtc->cursor.y--;
            vtc->cursor.x = vtc->width - 1;
        }

        if (vtc->on_move)
        {
            vtc->on_move(vtc, &vtc->cursor);
        }

        int i = (vtc->width * vtc->cursor.y) + vtc->cursor.x;
        vtcell_t *cell = &vtc->buffer[i];
        cell->attr = VTC_DEFAULT_ATTR;
        cell->c = ' ';
        vtc->on_paint(vtc, &vtc->buffer[i], i % vtc->width, i / vtc->width);
    }
    else
    {
        if (vtc->cursor.x >= vtc->width)
            vtconsole_newline(vtc);

        vtcell_t *cell =
            &vtc->buffer[vtc->cursor.x + vtc->cursor.y * vtc->width];
        cell->c = c;
        cell->attr = vtc->attr;

        if (vtc->on_paint)
        {
            vtc->on_paint(vtc, cell, vtc->cursor.x, vtc->cursor.y);
        }

        vtc->cursor.x++;

        if (vtc->on_move)
        {
            vtc->on_move(vtc, &vtc->cursor);
        }
    }
}

// Helper function for vtconsole_process to move the cursor P1 rows up
void vtconsole_csi_cuu(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.y = MAX(MIN(vtc->cursor.y - attr, vtc->height - 1), 1);
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Helper function for vtconsole_process to move the cursor P1 columns left
void vtconsole_csi_cud(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.y = MAX(MIN(vtc->cursor.y + attr, vtc->height - 1), 1);
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Helper function for vtconsole_process to move the cursor P1 columns right
void vtconsole_csi_cuf(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.x = MAX(MIN(vtc->cursor.x + attr, vtc->width - 1), 1);
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Helper function for vtconsole_process to move the cursor P1 rows down
void vtconsole_csi_cub(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.x = MAX(MIN(vtc->cursor.x - attr, vtc->width - 1), 1);
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Helper function for vtconsole_process to place the cursor to the first
// column of line P1 rows down from current
void vtconsole_csi_cnl(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.y = MAX(MIN(vtc->cursor.y + attr, vtc->height - 1), 1);
        vtc->cursor.x = 0;
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Helper function for vtconsole_process to place the cursor to the first
// column of line P1 rows up from current
void vtconsole_csi_cpl(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.y = MAX(MIN(vtc->cursor.y - attr, vtc->height - 1), 1);
        vtc->cursor.x = 0;
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Helper function of vtconsole_process to move the cursor to column P1
void vtconsole_csi_cha(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && !stack[0].empty)
    {
        int attr = stack[0].value;
        vtc->cursor.y = MAX(MIN(attr, vtc->height - 1), 1);
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Moves the cursor to row n, column m. The values are 1-based,
void vtconsole_csi_cup(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count == 1 && stack[0].empty)
    {
        vtc->cursor.x = 0;
        vtc->cursor.y = 0;
    }
    else if (count == 2)
    {
        if (stack[0].empty)
        {
            vtc->cursor.y = 0;
        }
        else
        {
            vtc->cursor.y = MIN(stack[0].value - 1, vtc->height - 1);
        }

        if (stack[1].empty)
        {
            vtc->cursor.y = 0;
        }
        else
        {
            vtc->cursor.x = MIN(stack[1].value - 1, vtc->width - 1);
        }
    }

    if (vtc->on_move)
    {
        vtc->on_move(vtc, &vtc->cursor);
    }
}

// Clears part of the screen.
void vtconsole_csi_ed(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    (void)(count);

    vtcursor_t cursor = vtc->cursor;

    if (stack[0].empty)
    {
        vtconsole_clear(vtc, cursor.x, cursor.y, vtc->width, vtc->height - 1);
    }
    else
    {
        int attr = stack[0].value;

        if (attr == 0)
            vtconsole_clear(vtc, cursor.x, cursor.y, vtc->width,
                            vtc->height - 1);
        else if (attr == 1)
            vtconsole_clear(vtc, 0, 0, cursor.x, cursor.y);
        else if (attr == 2)
            vtconsole_clear(vtc, 0, 0, vtc->width, vtc->height - 1);
    }
}

// Erases part of the line.
void vtconsole_csi_el(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    (void)(count);

    vtcursor_t cursor = vtc->cursor;

    if (stack[0].empty)
    {
        vtconsole_clear(vtc, cursor.x, cursor.y, vtc->width, cursor.y);
    }
    else
    {
        int attr = stack[0].value;

        if (attr == 0)
            vtconsole_clear(vtc, cursor.x, cursor.y, vtc->width, cursor.y);
        else if (attr == 1)
            vtconsole_clear(vtc, 0, cursor.y, cursor.x, cursor.y);
        else if (attr == 2)
            vtconsole_clear(vtc, 0, cursor.y, vtc->width, cursor.y);
    }
}

// Sets the appearance of the following characters
void vtconsole_csi_sgr(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (stack[i].empty || stack[i].value == 0)
        {
            vtc->attr = VTC_DEFAULT_ATTR;
        }
        else
        {
            int attr = stack[i].value;

            if (attr == 1) // Increased intensity
            {
                vtc->attr.bright = 1;
            }
            else if (attr >= 30 && attr <= 37) // Set foreground color
            {
                vtc->attr.fg = attr - 30;
            }
            else if (attr >= 40 && attr <= 47) // Set background color
            {
                vtc->attr.bg = attr - 40;
            }
        }
    }
}

void vtconsole_csi_l(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count != 1)
    {
        return;
    }
    if (stack[0].empty || stack[0].value != 25)
    {
        return;
    }

    vga_disable_cursor();
}

void vtconsole_csi_h(vtconsole_t *vtc, vtansi_arg_t *stack, int count)
{
    if (count != 1)
    {
        return;
    }

    if (stack[0].empty || stack[0].value != 25)
    {
        return;
    }

    vga_enable_cursor();
}

// vtconsole_append is called by vtconsole_process to process and print the
// keys pressed onto the console.
void vtconsole_process(vtconsole_t *vtc, char c)
{
    vtansi_parser_t *parser = &vtc->ansiparser;

    switch (parser->state)
    {
    case VTSTATE_ESC:
        if (c == '\033')
        {
            parser->state = VTSTATE_BRACKET;

            parser->index = 0;

            parser->stack[parser->index].value = 0;
            parser->stack[parser->index].empty = 1;
        }
        else
        {
            parser->state = VTSTATE_ESC;
            vtconsole_append(vtc, c);
        }
        break;

    case VTSTATE_BRACKET:
        if (c == '[')
        {
            parser->state = VTSTATE_ATTR;
        }
        else
        {
            parser->state = VTSTATE_ESC;
            vtconsole_append(vtc, c);
        }
        break;
    case VTSTATE_ATTR:
        if (c >= '0' && c <= '9')
        {
            parser->stack[parser->index].value *= 10;
            parser->stack[parser->index].value += (c - '0');
            parser->stack[parser->index].empty = 0;
        }
        else if (c == '?')
        {
            /* questionable (aka wrong) */
            break;
        }
        else
        {
            if ((parser->index) < VTC_ANSI_PARSER_STACK_SIZE)
            {
                parser->index++;
            }

            parser->stack[parser->index].value = 0;
            parser->stack[parser->index].empty = 1;

            parser->state = VTSTATE_ENDVAL;
        }
        break;
    default:
        break;
    }

    if (parser->state == VTSTATE_ENDVAL)
    {
        if (c == ';')
        {
            parser->state = VTSTATE_ATTR;
        }
        else
        {
            switch (c)
            {
            case 'A':
                /* Cursor up P1 rows */
                vtconsole_csi_cuu(vtc, parser->stack, parser->index);
                break;
            case 'B':
                /* Cursor down P1 rows */
                vtconsole_csi_cub(vtc, parser->stack, parser->index);
                break;
            case 'C':
                /* Cursor right P1 columns */
                vtconsole_csi_cuf(vtc, parser->stack, parser->index);
                break;
            case 'D':
                /* Cursor left P1 columns */
                vtconsole_csi_cud(vtc, parser->stack, parser->index);
                break;
            case 'E':
                /* Cursor to first column of line P1 rows down from current
                     */
                vtconsole_csi_cnl(vtc, parser->stack, parser->index);
                break;
            case 'F':
                /* Cursor to first column of line P1 rows up from current */
                vtconsole_csi_cpl(vtc, parser->stack, parser->index);
                break;
            case 'G':
                /* Cursor to column P1 */
                vtconsole_csi_cha(vtc, parser->stack, parser->index);
                break;
            case 'd':
                /* Cursor left P1 columns */
                break;
            case 'H':
                /* Moves the cursor to row n, column m. */
                vtconsole_csi_cup(vtc, parser->stack, parser->index);
                break;
            case 'J':
                /* Clears part of the screen. */
                vtconsole_csi_ed(vtc, parser->stack, parser->index);
                break;
            case 'K':
                /* Erases part of the line. */
                vtconsole_csi_el(vtc, parser->stack, parser->index);
                break;
            case 'm':
                /* Sets the appearance of the following characters */
                vtconsole_csi_sgr(vtc, parser->stack, parser->index);
                break;
            case 'l':
                vtconsole_csi_l(vtc, parser->stack, parser->index);
                break;
            case 'h':
                vtconsole_csi_h(vtc, parser->stack, parser->index);
                break;
            }

            parser->state = VTSTATE_ESC;
        }
    }
}

// vtconosle_putchar is called from vterminal_key_pressed
void vtconsole_putchar(vtconsole_t *vtc, char c) { vtconsole_process(vtc, c); }

// vtconsole_write is called from vterminal_write
void vtconsole_write(vtconsole_t *vtc, const char *buffer, uint32_t size)
{
    // looping through the whole size of the buffer
    for (uint32_t i = 0; i < size; i++)
    {
        // acquiting the ldisc associated with the vtconsole/vterminal
        ldisc_t *new_ldisc = &vterminal_to_tty(vtc)->tty_ldisc;

        // checking if the buffer is a backspsace and the last entered character was a tab
        if (buffer[i] == '\b' && new_ldisc->ldisc_buffer[(new_ldisc->ldisc_head)] == '\t')
        {
            // calling vtcomsole_process 'n' number of times.
            // where 'n' is the size of the tab.
            for (int j = 0; j < vtc->tabs[(vtc->tab_index - 1) % LDISC_BUFFER_SIZE]; j++)
            {
                vtconsole_process(vtc, buffer[i]);
            }
            vtc->tab_index--;
        }
        else
        {
            vtconsole_process(vtc, buffer[i]);
        }
    }
}

// called by vterminal_make_active to redraw the console.
void vtconsole_redraw(vtconsole_t *vtc)
{
    for (int i = 0; i < (vtc->width * vtc->height); i++)
    {
        if (vtc->on_paint)
        {
            vtc->on_paint(vtc, &vtc->buffer[i], i % vtc->width, i / vtc->width);
        }
    }
}

#define VGA_COLOR(__fg, __bg) (__bg << 4 | __fg)
#define VGA_ENTRY(__c, __fg, __bg) \
    ((((__bg)&0XF) << 4 | ((__fg)&0XF)) << 8 | ((__c)&0XFF))

// helper function for paint_callback.
void vga_cell(unsigned int x, unsigned int y, unsigned short entry)
{
    if (x < VGA_SCREEN_WIDTH)
    {
        if (y < VGA_SCREEN_WIDTH)
        {
            vga_write_char_at(y, x, entry);
        }
    }
}

static char colors[] = {
    [VTCOLOR_BLACK] = VGACOLOR_BLACK,
    [VTCOLOR_RED] = VGACOLOR_RED,
    [VTCOLOR_GREEN] = VGACOLOR_GREEN,
    [VTCOLOR_YELLOW] = VGACOLOR_BROWN,
    [VTCOLOR_BLUE] = VGACOLOR_BLUE,
    [VTCOLOR_MAGENTA] = VGACOLOR_MAGENTA,
    [VTCOLOR_CYAN] = VGACOLOR_CYAN,
    [VTCOLOR_GREY] = VGACOLOR_LIGHT_GRAY,
};

static char brightcolors[] = {
    [VTCOLOR_BLACK] = VGACOLOR_GRAY,
    [VTCOLOR_RED] = VGACOLOR_LIGHT_RED,
    [VTCOLOR_GREEN] = VGACOLOR_LIGHT_GREEN,
    [VTCOLOR_YELLOW] = VGACOLOR_LIGHT_YELLOW,
    [VTCOLOR_BLUE] = VGACOLOR_LIGHT_BLUE,
    [VTCOLOR_MAGENTA] = VGACOLOR_LIGHT_MAGENTA,
    [VTCOLOR_CYAN] = VGACOLOR_LIGHT_CYAN,
    [VTCOLOR_GREY] = VGACOLOR_WHITE,
};

static vterminal_t *active_vt = NULL;

// used for initializing the vtconsoles.
void paint_callback(vtconsole_t *vtc, vtcell_t *cell, int x, int y)
{
    if (vtc != active_vt)
    {
        return;
    }

    if (cell->attr.bright)
    {
        vga_cell(x, y,
                 VGA_ENTRY(cell->c, brightcolors[cell->attr.fg],
                           colors[cell->attr.bg]));
    }
    else
    {
        vga_cell(
            x, y,
            VGA_ENTRY(cell->c, colors[cell->attr.fg], colors[cell->attr.bg]));
    }
}

// used for initializing the vtconsoles.
void cursor_move_callback(vtconsole_t *vtc, vtcursor_t *cur)
{
    if (vtc != active_vt)
    {
        return;
    }
    vga_set_cursor(cur->y, cur->x);
}

// initialization function for vterminal which calls the vtconsole constructor
void vterminal_init(vtconsole_t *vt)
{
    vtconsole(vt, VGA_SCREEN_WIDTH, VGA_SCREEN_HEIGHT, paint_callback,
              cursor_move_callback);
}

// Used in tty.c to make a vterminal active and working.
void vterminal_make_active(vterminal_t *vt)
{
    active_vt = vt;
    vtconsole_redraw(vt);
    vga_set_cursor(vt->cursor.y, vt->cursor.x);
}

// called by ldisc_key_pressed from ldisc.c
void vterminal_key_pressed(vterminal_t *vt)
{
    char buf[LDISC_BUFFER_SIZE];
    size_t len =
        ldisc_get_current_line_raw(&vterminal_to_tty(vt)->tty_ldisc, buf);
    vtconsole_putchar(vt, buf[len - 1]);
}

void vterminal_scroll_to_bottom(vterminal_t *vt) { KASSERT(0); }

// ldisc_key_pressed calls this vterminal_write if VGA_BUF is not specified.
size_t vterminal_write(vterminal_t *vt, const char *buf, size_t len)
{
    vtconsole_write(vt, buf, len);
    return len;
}

// could be used in ldisc_key_pressed
size_t vterminal_echo_input(vterminal_t *vt, const char *buf, size_t len)
{
    vtconsole_write(vt, buf, len);
    return len;
}
