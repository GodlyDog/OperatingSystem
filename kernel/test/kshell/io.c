#include "test/kshell/io.h"
#include "util/debug.h"

#include "priv.h"

#ifndef __VFS__

#include "drivers/chardev.h"

#endif

#ifdef __VFS__

#include "fs/vfs_syscall.h"

#endif

#include "util/printf.h"
#include "util/string.h"

/*
 * If VFS is enabled, we can just use the syscalls.
 *
 * If VFS is not enabled, then we need to explicitly call the byte
 * device.
 */

#ifdef __VFS__

long kshell_write(kshell_t *ksh, const void *buf, size_t nbytes)
{
    long retval = do_write(ksh->ksh_out_fd, buf, nbytes);
    KASSERT(retval < 0 || (size_t)retval == nbytes);
    return retval;
}

long kshell_read(kshell_t *ksh, void *buf, size_t nbytes)
{
    return do_read(ksh->ksh_in_fd, buf, nbytes);
}

long kshell_write_all(kshell_t *ksh, void *buf, size_t nbytes)
{
    /* See comment in kshell_write */
    return kshell_write(ksh, buf, nbytes);
}

#else

long kshell_read(kshell_t *ksh, void *buf, size_t nbytes)
{
    return ksh->ksh_cd->cd_ops->read(ksh->ksh_cd, 0, buf, nbytes);
}

long kshell_write(kshell_t *ksh, const void *buf, size_t nbytes)
{
    return ksh->ksh_cd->cd_ops->write(ksh->ksh_cd, 0, buf, nbytes);
}

#endif

void kprint(kshell_t *ksh, const char *fmt, va_list args)
{
    char buf[KSH_BUF_SIZE];
    size_t count;

    vsnprintf(buf, sizeof(buf), fmt, args);
    count = strnlen(buf, sizeof(buf));
    kshell_write(ksh, buf, count);
}

void kprintf(kshell_t *ksh, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    kprint(ksh, fmt, args);
    va_end(args);
}
