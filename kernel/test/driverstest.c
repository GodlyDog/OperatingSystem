#include "errno.h"
#include "globals.h"

#include "test/usertest.h"
#include "test/proctest.h"

#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"

#include "proc/proc.h"
#include "proc/kthread.h"
#include "proc/sched.h"

#include "drivers/tty/tty.h"
#include "drivers/dev.h"
#include "drivers/blockdev.h"
#include "drivers/keyboard.h"

#define TEST_STR_1 "hello\n"
#define TEST_STR_2 "different string\n"
#define TEST_STR_3 "test"
#define TEST_BUF_SZ 10
#define NUM_PROCS 3
#define BLOCK_NUM 0

/*
    Tests inputting a character and a newline character 
*/
long test_basic_line_discipline(chardev_t* cd, tty_t* tty, ldisc_t* ldisc) { 
    ldisc_key_pressed(ldisc, 't'); 

    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_tail] == 't', 
                "character not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head != ldisc->ldisc_cooked && ldisc->ldisc_tail != ldisc->ldisc_head, 
                "pointers are not updated correctly");

    size_t previous_head_val = ldisc->ldisc_head; 
    ldisc_key_pressed(ldisc, '\n'); 
    test_assert(ldisc->ldisc_head == previous_head_val + 1, 
                "ldisc_head should have been incremented past newline character");
    test_assert(ldisc->ldisc_cooked == ldisc->ldisc_head, 
                "ldisc_cooked should be equal to ldisc_head"); 

    // reset line discipline for other tests before returning 
    ldisc->ldisc_head = ldisc->ldisc_cooked = ldisc->ldisc_tail = 0; 
    return 0; 
}

/*
    Tests inputting special characters
*/
long test_special_line_discipline(chardev_t* cd, tty_t* tty, ldisc_t* ldisc) {
    // test ETX
    ldisc_key_pressed(ldisc, 't');
    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_tail] == 't', 
                "character not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head != ldisc->ldisc_cooked && ldisc->ldisc_tail != ldisc->ldisc_head, 
                "pointers are not updated correctly");

    size_t previous_head_val = ldisc->ldisc_head; 
    ldisc_key_pressed(ldisc, ETX);
    test_assert(ldisc->ldisc_head == previous_head_val, 
                "ldisc_head should have been adjusted to just after a newline character");
    test_assert(ldisc->ldisc_cooked == ldisc->ldisc_head, 
                "ldisc_cooked should be equal to ldisc_head"); 

    ldisc->ldisc_head = ldisc->ldisc_cooked = ldisc->ldisc_tail = 0;

    // test EOT
    ldisc_key_pressed(ldisc, 'a');
    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_tail] == 'a', 
                "character not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head != ldisc->ldisc_cooked && ldisc->ldisc_tail != ldisc->ldisc_head, 
                "pointers are not updated correctly");
    ldisc_key_pressed(ldisc, 'a');
    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_tail] == 'a', 
                "character not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head != ldisc->ldisc_cooked && ldisc->ldisc_tail != ldisc->ldisc_head, 
                "pointers are not updated correctly");
    previous_head_val = ldisc->ldisc_head;
    ldisc_key_pressed(ldisc, EOT);
    test_assert(ldisc->ldisc_head == previous_head_val + 1, 
                "ldisc_head should have been incremented by one");
    test_assert(ldisc->ldisc_cooked == ldisc->ldisc_head, 
                "ldisc_cooked should be equal to ldisc_head");
    ldisc->ldisc_head = ldisc->ldisc_cooked = ldisc->ldisc_tail = 0;

    // test BS
    ldisc_key_pressed(ldisc, 'a');
    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_tail] == 'a', 
                "character not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head != ldisc->ldisc_cooked && ldisc->ldisc_tail != ldisc->ldisc_head, 
                "pointers are not updated correctly");
    ldisc_key_pressed(ldisc, 'b');
    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_head - 1] == 'b', 
                "character not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head != ldisc->ldisc_cooked && ldisc->ldisc_tail != ldisc->ldisc_head, 
                "pointers are not updated correctly");
    previous_head_val = ldisc->ldisc_head;
    ldisc_key_pressed(ldisc, BS);
    test_assert(ldisc->ldisc_head == previous_head_val - 1, 
                "ldisc_head should have been decremented by one");
    test_assert(ldisc->ldisc_cooked != ldisc->ldisc_head, 
                "ldisc_cooked should not be equal to ldisc_head");
    test_assert(ldisc->ldisc_buffer[ldisc->ldisc_head - 1] == 'a',
                "ldisc_head is not in the correct location");
    ldisc->ldisc_head = ldisc->ldisc_cooked = ldisc->ldisc_tail = 0;
    return 0; 
}

/*
    Tests inputting a character and a newline character 
*/
long test_line_discipline_overflow(chardev_t* cd, tty_t* tty, ldisc_t* ldisc) {
    for (int i = 0; i < LDISC_BUFFER_SIZE * 4; i++) {
        ldisc_key_pressed(ldisc, 't'); 
    }
    test_assert(ldisc->ldisc_buffer[LDISC_BUFFER_SIZE - 2] == 't', 
                "characters not inputted into buffer correctly"); 
    test_assert(ldisc->ldisc_head == LDISC_BUFFER_SIZE - 1, 
                "ldisc head has overflowed");
    test_assert(ldisc->ldisc_buffer[LDISC_BUFFER_SIZE - 1] != 't',
                "ldisc buffer has overflowed");
    ldisc_key_pressed(ldisc, '\n'); 
    test_assert(ldisc->ldisc_head == 0, 
                "ldisc_head should have been incremented to 0");
    test_assert(ldisc->ldisc_cooked == ldisc->ldisc_head, 
                "ldisc_cooked should be equal to ldisc_head"); 

    ldisc->ldisc_head = ldisc->ldisc_cooked = ldisc->ldisc_tail = 0; 
    return 0; 
}

long driverstest_main(long arg1, void* arg2)
{
    dbg(DBG_TEST, "\nStarting Drivers tests\n");
    test_init();
    chardev_t* cd = chardev_lookup(MKDEVID(TTY_MAJOR, 0)); 
    tty_t* tty = cd_to_tty(cd); 
    ldisc_t* ldisc = &tty->tty_ldisc; 
    test_basic_line_discipline(cd, tty, ldisc);
    test_special_line_discipline(cd, tty, ldisc);
    test_line_discipline_overflow(cd, tty, ldisc);
    ldisc->ldisc_head = ldisc->ldisc_cooked = ldisc->ldisc_tail = 0; 
    test_fini();
    return 0;
}