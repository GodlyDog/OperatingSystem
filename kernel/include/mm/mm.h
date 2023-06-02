#pragma once

#define MM_POISON 1
#define MM_POISON_ALLOC 0xBB
#define MM_POISON_FREE 0xDD

#define USER_MEM_LOW 0x00400000   /* inclusive */
#define USER_MEM_HIGH (1UL << 47) /* exclusive */
