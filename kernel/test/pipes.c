#include "errno.h"
#include "globals.h"

#include "fs/file.h"
#include "fs/pipe.h"
#include "fs/vfs_syscall.h"

#include "test/kshell/io.h"
#include "test/kshell/kshell.h"

#define IMAX 256
#define JMAX 16
#define KMAX 16
#define ISTEP (JMAX * KMAX)

static kthread_t *make_proc_and_thread(char *name, kthread_func_t func,
                                       int arg1, void *arg2)
{
    proc_t *proc = proc_create(name);
    if (!proc)
    {
        return NULL;
    }

    int i;
    for (i = 0; i < NFILES; ++i)
    {
        proc->p_files[i] = curproc->p_files[i];
        if (proc->p_files[i])
        {
            fref(proc->p_files[i]);
        }
    }
    return kthread_create(proc, func, arg1, arg2);
}

static void *producer(long arg1, void *arg2)
{
    int fd = (int)arg1;
    kshell_t *ksh = (kshell_t *)arg2;

    kprintf(ksh, "Producing bytes...\n");

    unsigned char buf[KMAX];
    int i, j, k;
    for (i = 0; i < IMAX; ++i)
    {
        for (j = 0; j < JMAX; ++j)
        {
            for (k = 0; k < KMAX; ++k)
            {
                buf[k] = (unsigned char)(i ^ (j * KMAX + k));
            }
            kprintf(ksh, "Writing bytes %d to %d\n", i * ISTEP + j * KMAX,
                    i * ISTEP + (j + 1) * KMAX);
            if (do_write(fd, buf, KMAX) == -EPIPE)
            {
                kprintf(ksh, "Got EPIPE\n");
                goto out;
            }
        }
        kprintf(ksh, "Wrote %d bytes\n", (i + 1) * ISTEP);
    }
out:
    return NULL;
}

static void *consumer(long arg1, void *arg2)
{
    int fd = (int)arg1;
    kshell_t *ksh = (kshell_t *)arg2;

    kprintf(ksh, "Consuming bytes...\n");
    unsigned char buf[KMAX];
    int i, j, k;
    for (i = 0; i < IMAX; ++i)
    {
        for (j = 0; j < JMAX; ++j)
        {
            kprintf(ksh, "Reading bytes %d to %d\n", i * ISTEP + j * KMAX,
                    i * ISTEP + (j + 1) * KMAX);
            if (do_read(fd, buf, KMAX) == 0)
            {
                kprintf(ksh, "End of pipe\n");
                goto out;
            }
            for (k = 0; k < KMAX; ++k)
            {
                if (buf[k] != (i ^ (j * KMAX + k)))
                {
                    kprintf(ksh, "Byte %d incorrect (expected %2x, got %2x)\n",
                            i * ISTEP + j * KMAX + k, (i ^ (j * KMAX + k)),
                            buf[k]);
                }
            }
        }
        kprintf(ksh, "Read %d bytes\n", (i + 1) * ISTEP);
    }
out:
    return NULL;
}

static int test_pipes(kshell_t *ksh, int argc, char **argv)
{
    int pfds[2];
    int err = do_pipe(pfds);
    if (err < 0)
    {
        kprintf(ksh, "Failed to create pipe\n");
    }
    kprintf(ksh, "Created pipe with read fd %d and write fd %d\n", pfds[0],
            pfds[1]);

    sched_make_runnable(
        make_proc_and_thread("producer", producer, pfds[1], ksh));
    kprintf(ksh, "Created producer process\n");
    sched_make_runnable(
        make_proc_and_thread("consumer", consumer, pfds[0], ksh));
    kprintf(ksh, "Created consumer process\n");

    do_waitpid(-1, 0, 0);
    do_waitpid(-1, 0, 0);
    return 0;
}

#ifdef __PIPES__
static __attribute__((unused)) void test_pipes_init()
{
    kshell_add_command("test_pipes", test_pipes, "run pipe tests");
}
init_func(test_pipes_init);
init_depends(kshell_init);
#endif /* __PIPES__ */
