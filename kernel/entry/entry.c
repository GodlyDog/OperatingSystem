/* entry.c */
#include "main/entry.h"
#include "types.h"

#include "multiboot.h"

struct multiboot_tag *mb_tag;

void entry(void *bootinfo_addr)
{
    mb_tag = bootinfo_addr;
    kmain();
    __asm__("cli\n\thlt");
}
