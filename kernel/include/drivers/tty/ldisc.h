#pragma once

#include "types.h"
#include <proc/kmutex.h>

#define LDISC_BUFFER_SIZE 128

/**
 * The line discipline is implemented as a circular buffer containing two 
 * sections: cooked and raw. These sections are tracked by three indices: 
 * ldisc_cooked, ldisc_tail, and ldisc_head.
 *  
 * New characters (via ldisc_key_pressed) are put at the head position (and the 
 * head is incremented). If a newline is received, cooked is moved up to the head. 
 * Characters are read from tail up until cooked, and the tail is updated
 * to equal cooked.
 * 
 * The cooked portion (ready for reading) runs from ldisc_tail (inclusive) to
 * ldisc_cooked (exclusive). The raw portion (subject to editing) runs from 
 * ldisc_cooked (inclusive) to ldisc_head (exclusive).
 * 
 * e.g.
 *            [..........t........c...h.......]
 *              (cooked) ^^^^^^^^^
 *                                ^^^^ (raw)
 *
 * Bear in mind that the buffer is circular, so another possible configuration
 * might be 
 * 
 *            [....h............t......c......]
 *                     (cooked) ^^^^^^^
 *             ^^^^                    ^^^^^^^ (raw)
 * 
 * When incrementing the indices, make sure that you take the circularity of
 * the buffer into account! (Hint: using LDISC_BUFFER_SIZE macro will be helpful.) 
 * 
 * The field ldisc_full is used to indicate when the circular buffer has been
 * completely filled. This is necessary because there are two possible states
 * in which cooked == tail == head: 
 * 
 *      1) The buffer is empty, or 
 * 
 *      2) The buffer is full: head has wrapped around and is equal to tail. 
 * 
 * ldisc_full is used to differentiate between these two states.
 */
typedef struct ldisc
{
    size_t ldisc_cooked; // Cooked is the index after the most last or most recent '\n' in the buffer.
    size_t ldisc_tail;   // Tail is the index from which characters are read by processes
    size_t ldisc_head;   // Head is the index at which a new character is placed
    char ldisc_full;     // Full identifies if the buffer is full
                         // 1 -> full
                         // 0 -> not full  

    ktqueue_t ldisc_read_queue; // Queue for threads waiting for data to be read 
    char ldisc_buffer[LDISC_BUFFER_SIZE];
} ldisc_t;

void ldisc_init(ldisc_t *ldisc);

long ldisc_wait_read(ldisc_t *ldisc, spinlock_t *lock);

size_t ldisc_read(ldisc_t *ldisc, char *buf, size_t count);

void ldisc_key_pressed(ldisc_t *ldisc, char c);

size_t ldisc_get_current_line_raw(ldisc_t *ldisc, char *s);