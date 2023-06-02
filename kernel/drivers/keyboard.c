#include "drivers/keyboard.h"

#include "drivers/tty/tty.h"

#include "main/interrupt.h"
#include "main/io.h"

#define IRQ_KEYBOARD 1

/* Indicates that one of these is "being held down" */
#define SHIFT_MASK 0x1
#define CTRL_MASK 0x2
/* Indicates that an escape code was the previous key received */
#define ESC_MASK 0x4
static int curmask = 0;

/* Where to read from to get scancodes */
#define KEYBOARD_IN_PORT 0x60
#define KEYBOARD_CMD_PORT 0x61

/* Scancodes for special keys */
#define LSHIFT 0x2a
#define RSHIFT 0x36
#define CTRL 0x1d
/* Right ctrl is escaped */
/* Our keyboard driver totally ignores ALT */

#define ESC0 0xe0
#define ESC1 0xe1

/* If the scancode & BREAK_MASK, it's a break code; otherwise, it's a make code
 */
#define BREAK_MASK 0x80

#define NORMAL_KEY_HIGH 0x39

/* Some sneaky value to indicate we don't actually pass anything to the terminal
 */
#define NO_CHAR 0xff

#define F1_SCANCODE 0x3b
#define F12_SCANCODE (F1_SCANCODE + 11)

/* Scancode tables copied from
   http://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html */

/* The scancode table for "normal" scancodes - from 02 to 39 */
/* Unsupported chars are symbolized by \0 */
static const char *normal_scancodes =
    "\0"               /* Error */
    "\e"               /* Escape key */
    "1234567890-="     /* Top row */
    "\b"               /* Backspace */
    "\tqwertyuiop[]\n" /* Next row - ish */
    "\0"               /* Left ctrl */
    "asdfghjkl;\'`"
    "\0" /* Lshift */
    "\\"
    "zxcvbnm,./"
    "\0\0\0" /* Rshift, prtscrn, Lalt */
    " ";     /* Space bar */
/* As above, but if shift is pressed */
static const char *shift_scancodes =
    "\0"
    "\e"
    "!@#$%^&*()_+"
    "\b"
    "\tQWERTYUIOP{}\n"
    "\0"
    "ASDFGHJKL:\"~"
    "\0"
    "|"
    "ZXCVBNM<>?"
    "\0\0\0"
    " ";

static keyboard_char_handler_t keyboard_handler = NULL;

/* This is the function we register with the interrupt handler - it reads the
 * scancode and, if appropriate, call's the tty's receive_char function */
static long keyboard_intr_handler(regs_t *regs)
{
    uint8_t sc;     /* The scancode we receive */
    int break_code; /* Was it a break code */
    /* the resulting character ('\0' -> ignored char) */
    uint8_t c = NO_CHAR;
    /* Get the scancode */
    sc = inb(KEYBOARD_IN_PORT);
    /* Separate out the break code */
    break_code = sc & BREAK_MASK;
    sc &= ~BREAK_MASK;

    /* dbg(DBG_KB, ("scancode 0x%x, break 0x%x\n", sc, break_code)); */

    /* The order of this conditional is very, very tricky - be careful when
     * editing! */

    /* Most break codes are ignored */
    if (break_code)
    {
        /* Shift/ctrl release */
        if (sc == LSHIFT || sc == RSHIFT)
        {
            curmask &= ~SHIFT_MASK;
        }
        else if (sc == CTRL)
        {
            curmask &= ~CTRL_MASK;
        }
    }
    /* Check for the special keys */
    else if (sc == LSHIFT || sc == RSHIFT)
    {
        curmask |= SHIFT_MASK;
    }
    else if (sc == CTRL)
    {
        curmask |= CTRL_MASK;
    }
    /* All escaped keys past this point (anything except right shift and right
     * ctrl) will be ignored */
    else if (curmask & ESC_MASK)
    {
        /* Escape mask only lasts for one key */
        curmask &= ~ESC_MASK;
    }
    /* Now check for escape code */
    else if (sc == ESC0 || sc == ESC1)
    {
        curmask |= ESC_MASK;
    }

    else if (sc >= F1_SCANCODE && sc <= F12_SCANCODE)
    {
        c = (uint8_t)(F1 + (sc - F1_SCANCODE));
    }
    /* Check for Ctrl+Backspace which indicates scroll down */
    else if ((curmask & CTRL_MASK) && (curmask & SHIFT_MASK) &&
             sc == SCROLL_DOWN)
    {
        c = SCROLL_DOWN_PAGE;
    }

    else if ((curmask & CTRL_MASK) && (curmask & SHIFT_MASK) &&
             sc == SCROLL_UP)
    {
        c = SCROLL_UP_PAGE;
    }

    else if ((curmask & CTRL_MASK) && sc == SCROLL_DOWN)
    {
        c = SCROLL_DOWN;
    }
    /* Check for Ctrl+Enter which indicates scroll down */
    else if ((curmask & CTRL_MASK) && sc == SCROLL_UP)
    {
        c = SCROLL_UP;
    }
    /* Check to make sure the key isn't high enough that it won't be found in
     * tables */
    else if (sc > NORMAL_KEY_HIGH)
    {
        /* ignore */
    }
    /* Control characters */
    else if (curmask & CTRL_MASK)
    {
        /* Because of the way ASCII works, the control chars are based on the
         * values of the shifted chars produced without control */
        c = (uint8_t)shift_scancodes[sc];
        /* Range of chars that have corresponding control chars */
        if (c >= 0x40 && c < 0x60)
        {
            c -= 0x40;
        }
        else
        {
            c = NO_CHAR;
        }
    }
    /* Capitals */
    else if (curmask & SHIFT_MASK)
    {
        c = (uint8_t)shift_scancodes[sc];
    }
    else
    {
        c = (uint8_t)normal_scancodes[sc];
    }

    if (c != NO_CHAR)
    {
        keyboard_handler(c);
    }
    else
    {
        //        panic("get rid of me: char was: %c (%d) (%x)\n", c, c, c);
    }
    dbg(DBG_KB, "received scancode 0x%x; resolved to char 0x%x\n", sc, c);
    return 0;
}

void keyboard_init(keyboard_char_handler_t handler)
{
    intr_map(IRQ_KEYBOARD, INTR_KEYBOARD);
    intr_register(INTR_KEYBOARD, keyboard_intr_handler);
    keyboard_handler = handler;
}
