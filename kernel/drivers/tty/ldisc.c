#include "drivers/tty/ldisc.h"
#include <drivers/keyboard.h>
#include <drivers/tty/tty.h>
#include <errno.h>
#include <util/bits.h>
#include <util/debug.h>
#include <util/string.h>

#define ldisc_to_tty(ldisc) CONTAINER_OF((ldisc), tty_t, tty_ldisc)

/**
 * Initialize the line discipline. Don't forget to wipe the buffer associated
 * with the line discipline clean.
 *
 * @param ldisc line discipline.
 */
void ldisc_init(ldisc_t *ldisc)
{
    ldisc->ldisc_cooked = 0;
    ldisc->ldisc_tail = 0;
    ldisc->ldisc_head = 0;
    ldisc->ldisc_full = 0;
    sched_queue_init(&ldisc->ldisc_read_queue);
    for (int i = 0; i < LDISC_BUFFER_SIZE; i++) {
        ldisc->ldisc_buffer[i] = '\0';
    }
}

/**
 * Increments an ldisc buffer index using circular buffer logic
 * @param x index to increment
 * @returns size_t of x incremented by one
*/

size_t ldisc_increment(size_t x) {
    if (x == LDISC_BUFFER_SIZE - 1) {
        return 0;
    } else {
        return x + 1;
    }
}

/**
 * Decrements an ldisc buffer index using circular buffer logic
 * @param x index to decrement
 * @returns size_t of x decremented by one
*/

size_t ldisc_decrement(size_t x) {
    if (x == 0) {
        return LDISC_BUFFER_SIZE - 1;
    } else {
        return x - 1;
    }
}

/**
 * While there are no new characters to be read from the line discipline's
 * buffer, you should make the current thread to sleep on the line discipline's
 * read queue. Note that this sleep can be cancelled. What conditions must be met 
 * for there to be no characters to be read?
 *
 * @param  ldisc the line discipline
 * @param  lock  the lock associated with `ldisc`
 * @return       0 if there are new characters to be read or the ldisc is full.
 *               If the sleep was interrupted, return what
 *               `sched_cancellable_sleep_on` returned (i.e. -EINTR)
 */
long ldisc_wait_read(ldisc_t *ldisc, spinlock_t *lock)
{
    while (ldisc->ldisc_cooked == ldisc->ldisc_tail) {
         if (ldisc->ldisc_full) {
            return 0;
         }
         long status = sched_cancellable_sleep_on(&ldisc->ldisc_read_queue, lock);
         if (status < 0) {
            return status;
         }
    }
    return 0;
}

/**
 * Reads `count` bytes (at max) from the line discipline's buffer into the
 * provided buffer. Keep in mind the the ldisc's buffer is circular.
 *
 * If you encounter a new line symbol before you have read `count` bytes, you
 * should stop copying and return the bytes read until now.
 * 
 * If you encounter an `EOT` you should stop reading and you should NOT include 
 * the `EOT` in the count of the number of bytes read
 *
 * @param  ldisc the line discipline
 * @param  buf   the buffer to read into.
 * @param  count the maximum number of bytes to read from ldisc.
 * @return       the number of bytes read from the ldisc.
 */
size_t ldisc_read(ldisc_t *ldisc, char *buf, size_t count)
{
    size_t tail = ldisc->ldisc_tail;
    size_t cooked = ldisc->ldisc_cooked;
    size_t iterator = tail;
    size_t num_bytes = 0;
    while ((iterator != cooked) || ldisc->ldisc_full) {
        if (ldisc->ldisc_buffer[iterator] == EOT) {
            buf[num_bytes] = ldisc->ldisc_buffer[iterator];
            ldisc->ldisc_tail = ldisc_increment(iterator);
            return num_bytes;
        }
        buf[num_bytes] = ldisc->ldisc_buffer[iterator];
        iterator = ldisc_increment(iterator);
        num_bytes = num_bytes + 1;
        if (num_bytes > 0 && ldisc->ldisc_full == 1) {
            ldisc->ldisc_full = 0;
        }
        if (buf[num_bytes - 1] == '\n') {
            ldisc->ldisc_tail = iterator;
            return num_bytes;
        }
        if (num_bytes == count) {
            ldisc->ldisc_tail = iterator;
            return num_bytes;
        }
    }
    return num_bytes;
}

/**
 * Place the character received into the ldisc's buffer. You should also update
 * relevant fields of the struct.
 *
 * An easier way of handling new characters is making sure that you always have
 * one byte left in the line discipline. This way, if the new character you
 * received is a new line symbol (user hit enter), you can still place the new
 * line symbol into the buffer; if the new character is not a new line symbol,
 * you shouldn't place it into the buffer so that you can leave the space for
 * a new line symbol in the future. 
 * 
 * If the line discipline is full, unless the incoming character is a BS or 
 * ETX, it should not be handled and discarded. 
 *
 * Here are some special cases to consider:
 *      1. If the character is a backspace:
 *          * if there is a character to remove you must also emit a `\b` to
 *            the vterminal.
 *      2. If the character is end of transmission (EOT) character (typing ctrl-d)
 *      3. If the character is end of text (ETX) character (typing ctrl-c)
 *      4. If your buffer is almost full and what you received is not a new line
 *      symbol
 *
 * If you did receive a new line symbol, you should wake up the thread that is
 * sleeping on the wait queue of the line discipline. You should also
 * emit a `\n` to the vterminal by using `vterminal_write`.  
 * 
 * If you encounter the `EOT` character, you should add it to the buffer, 
 * cook the buffer, and wake up the reader (but do not emit an `\n` character 
 * to the vterminal)
 * 
 * In case of `ETX` you should cause the input line to be effectively transformed
 * into a cooked blank line. You should clear uncooked portion of the line, by 
 * adjusting ldisc_head. 
 *
 * Finally, if the none of the above cases apply you should fallback to
 * `vterminal_key_pressed`.
 *
 * Don't forget to write the corresponding characters to the virtual terminal
 * when it applies!
 *
 * @param ldisc the line discipline
 * @param c     the new character
 */
void ldisc_key_pressed(ldisc_t *ldisc, char c)
{
    if (ldisc->ldisc_full == 1) {
        if (c != ETX && c != BS) {
            return;
        }
    }
    if (ldisc->ldisc_head == ldisc_decrement(ldisc->ldisc_tail)) {
        if (c != '\n' && c != BS && c != ETX) {
            return;
        } else if (c == '\n') {
            ldisc->ldisc_full = 1;
        }
    }
    if (c == '\n') {
        ldisc->ldisc_buffer[ldisc->ldisc_head] = c;
        ldisc->ldisc_head = ldisc_increment(ldisc->ldisc_head);
        ldisc->ldisc_cooked = ldisc->ldisc_head;
        sched_wakeup_on(&ldisc->ldisc_read_queue, NULL);
        vterminal_write(&ldisc_to_tty(ldisc)->tty_vterminal, "\n", 1);
        return;
    }
    if (c == EOT) {
        ldisc->ldisc_buffer[ldisc->ldisc_head] = c;
        ldisc->ldisc_head = ldisc_increment(ldisc->ldisc_head);
        ldisc->ldisc_cooked = ldisc->ldisc_head;
        sched_wakeup_on(&ldisc->ldisc_read_queue, NULL);
        return;
    }
    if (c == ETX) {
        ldisc->ldisc_head = ldisc_increment(ldisc->ldisc_cooked);
        ldisc->ldisc_buffer[ldisc_decrement(ldisc->ldisc_head)] = '\n';
        ldisc->ldisc_cooked = ldisc->ldisc_head;
        // QUESTION: write something in this case?
        return;
    }
    if (c == BS) {
        if (ldisc->ldisc_head == ldisc->ldisc_cooked) {
            return;
        }
        ldisc->ldisc_head = ldisc_decrement(ldisc->ldisc_head);
        vterminal_write(&ldisc_to_tty(ldisc)->tty_vterminal, "\b", 1);
        return;
    }
    ldisc->ldisc_buffer[ldisc->ldisc_head] = c;
    ldisc->ldisc_head = ldisc_increment(ldisc->ldisc_head);
    vterminal_key_pressed(&ldisc_to_tty(ldisc)->tty_vterminal);
}

/**
 * Copy the raw part of the line discipline buffer into the buffer provided.
 *
 * @param  ldisc the line discipline
 * @param  s     the character buffer to write to
 * @return       the number of bytes copied
 */
size_t ldisc_get_current_line_raw(ldisc_t *ldisc, char *s)
{
    size_t cooked = ldisc->ldisc_cooked;
    size_t head = ldisc->ldisc_head;
    if (cooked == head) {
        return 0;
    }
    size_t iterator = cooked;
    size_t num_bytes = 0;
    while (iterator != head) {
        s[num_bytes] = ldisc->ldisc_buffer[iterator];
        iterator = ldisc_increment(iterator);
        num_bytes = num_bytes + 1;
    }
    return num_bytes;
}
