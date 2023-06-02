#pragma once

#include "test/kshell/kshell.h"

#define KSHELL_CMD(name) \
    long kshell_##name(kshell_t *ksh, size_t argc, char **argv)

KSHELL_CMD(help);

KSHELL_CMD(exit);

KSHELL_CMD(halt);

KSHELL_CMD(echo);

KSHELL_CMD(clear);

#ifdef __VFS__
KSHELL_CMD(cat);
KSHELL_CMD(ls);
KSHELL_CMD(cd);
KSHELL_CMD(rm);
KSHELL_CMD(link);
KSHELL_CMD(rmdir);
KSHELL_CMD(mkdir);
KSHELL_CMD(stat);
KSHELL_CMD(vfs_test);
#endif

#ifdef __S5FS__
KSHELL_CMD(s5fstest);
#endif
