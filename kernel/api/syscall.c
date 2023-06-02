#include "errno.h"
#include "globals.h"
#include "kernel.h"
#include <fs/vfs.h>
#include <util/time.h>

#include "main/inits.h"
#include "main/interrupt.h"

#include "mm/kmalloc.h"
#include "mm/mman.h"

#include "fs/vfs_syscall.h"
#include "fs/vnode.h"

#include "drivers/tty/tty.h"
#include "test/kshell/kshell.h"

#include "vm/brk.h"
#include "vm/mmap.h"

#include "api/access.h"
#include "api/exec.h"
#include "api/syscall.h"
#include "api/utsname.h"

static long syscall_handler(regs_t *regs);

static long syscall_dispatch(size_t sysnum, uintptr_t args, regs_t *regs);

extern size_t active_tty;

static const char *syscall_strings[49] = {
    "syscall", "exit", "fork", "read", "write", "open",
    "close", "waitpid", "link", "unlink", "execve", "chdir",
    "sleep", "unknown", "lseek", "sync", "nuke", "dup",
    "pipe", "ioctl", "unknown", "rmdir", "mkdir", "getdents",
    "mmap", "mprotect", "munmap", "rename", "uname", "thr_create",
    "thr_cancel", "thr_exit", "thr_yield", "thr_join", "gettid", "getpid",
    "unknown", "unkown", "unknown", "errno", "halt", "get_free_mem",
    "set_errno", "dup2", "brk", "mount", "umount", "stat", "usleep"};

void syscall_init(void) { intr_register(INTR_SYSCALL, syscall_handler); }

// if condition, set errno to err and return -1
#define ERROR_OUT(condition, err) \
    if (condition)                \
    {                             \
        curthr->kt_errno = (err); \
        return -1;                \
    }

// if ret < 0, set errno to -ret and return -1
#define ERROR_OUT_RET(ret) ERROR_OUT(ret < 0, -ret)

/*
 * Be sure to look at other examples of implemented system calls to see how
 * this should be done - the general outline is as follows.
 * 
 * - Initialize a read_args_t struct locally in kernel space and copy from 
 *   userland args. 
 * - Allocate a temporary buffer (a page-aligned block of n pages that are 
 *   enough space to store the number of bytes to read)
 * - Call do_read() with the buffer and then copy the buffer to the userland 
 *   args after the system call 
 * - Make sure to free the temporary buffer allocated
 * - Return the number of bytes read, or return -1 and set the current thread's
 *   errno appropriately using ERROR_OUT_RET. 
 */
static long sys_read(read_args_t *args)
{
    read_args_t arguments;
    long ret = copy_from_user(&arguments, args, sizeof(arguments));
    ERROR_OUT_RET(ret);
    size_t npages = 1;
    while (npages * PAGE_SIZE < arguments.nbytes) {
        npages += 1;
    }
    char* buffer = page_alloc_n(npages);
    if (!buffer) {
        ret = -ENOMEM;
        ERROR_OUT_RET(ret);
    }
    ret = do_read(arguments.fd, buffer, arguments.nbytes);
    ssize_t bytes_read = ret;
    if (bytes_read == 0) {
        return 0;
    }
    ERROR_OUT_RET(ret);
    ret = copy_to_user(arguments.buf, buffer, arguments.nbytes);
    ERROR_OUT_RET(ret);
    page_free_n(buffer, npages);
    return bytes_read;
}

/*
 * Be sure to look at other examples of implemented system calls to see how
 * this should be done - the general outline is as follows.
 *
 * This function is very similar to sys_read - see above comments. You'll need
 * to use the functions copy_from_user() and do_write(). Make sure to
 * allocate a new temporary buffer for the data that is being written. This
 * is to ensure that pagefaults within kernel mode do not happen. 
 */
static long sys_write(write_args_t *args)
{
    write_args_t arguments;
    long ret = copy_from_user(&arguments, args, sizeof(arguments));
    ERROR_OUT_RET(ret);
    size_t npages = 1;
    while (npages * PAGE_SIZE < arguments.nbytes) {
        npages += 1;
    }
    char* buffer = page_alloc_n(npages);
    if (!buffer) {
        ret = -ENOMEM;
        ERROR_OUT_RET(ret);
    }
    ret = copy_from_user(buffer, arguments.buf, arguments.nbytes);
    ret = do_write(arguments.fd, buffer, arguments.nbytes);
    ssize_t bytes_written = ret;
    ERROR_OUT_RET(ret);
    page_free_n(buffer, npages);
    return bytes_written;
}

/*
 * This similar to the other system calls that you have implemented above. 
 * 
 * The general steps are as follows: 
 *  - Copy the arguments from user memory 
 *  - Check that the count field is at least the size of a dirent_t
 *  - Use a while loop to read count / sizeof(dirent_t) directory entries into 
 *    the provided dirp and call do_getdent
 *  - Return the number of bytes read
 */
static long sys_getdents(getdents_args_t *args)
{
    getdents_args_t arguments;
    long ret = copy_from_user(&arguments, args, sizeof(arguments));
    ERROR_OUT_RET(ret);
    if (arguments.count < sizeof(dirent_t)) {
        ERROR_OUT_RET(-EINVAL); // What error message here?
    }
    size_t num_read = 0;
    while(arguments.count > (num_read * sizeof(dirent_t))) {
        dirent_t dirp;
        ssize_t read = do_getdent(arguments.fd, &dirp);
        ERROR_OUT_RET(read);
        if (read != sizeof(dirent_t)) {
            return num_read * sizeof(dirent_t);
        }
        ret = copy_to_user(arguments.dirp + num_read, &dirp, sizeof(dirent_t));
        num_read += 1;
        ERROR_OUT_RET(ret);
    }
    return num_read * sizeof(dirent_t);
}

#ifdef __MOUNTING__
static long sys_mount(mount_args_t *arg)
{
    mount_args_t kern_args;
    char *source;
    char *target;
    char *type;
    long ret;

    if (copy_from_user(&kern_args, arg, sizeof(kern_args)) < 0)
    {
        curthr->kt_errno = EFAULT;
        return -1;
    }

    /* null is okay only for the source */
    source = user_strdup(&kern_args.spec);
    if (NULL == (target = user_strdup(&kern_args.dir)))
    {
        kfree(source);
        curthr->kt_errno = EINVAL;
        return -1;
    }
    if (NULL == (type = user_strdup(&kern_args.fstype)))
    {
        kfree(source);
        kfree(target);
        curthr->kt_errno = EINVAL;
        return -1;
    }

    ret = do_mount(source, target, type);
    kfree(source);
    kfree(target);
    kfree(type);

    if (ret)
    {
        curthr->kt_errno = -ret;
        return -1;
    }

    return 0;
}

static long sys_umount(argstr_t *input)
{
    argstr_t kstr;
    char *target;
    long ret;

    if (copy_from_user(&kstr, input, sizeof(kstr)) < 0)
    {
        curthr->kt_errno = EFAULT;
        return -1;
    }

    if (NULL == (target = user_strdup(&kstr)))
    {
        curthr->kt_errno = EINVAL;
        return -1;
    }

    ret = do_umount(target);
    kfree(target);

    if (ret)
    {
        curthr->kt_errno = -ret;
        return -1;
    }

    return 0;
}
#endif

static long sys_close(int fd)
{
    long ret = do_close(fd);
    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_dup(int fd)
{
    long ret = do_dup(fd);
    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_dup2(const dup2_args_t *args)
{
    dup2_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);
    ret = do_dup2(kargs.ofd, kargs.nfd);
    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_mkdir(mkdir_args_t *args)
{
    mkdir_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *path;
    ret = user_strdup(&kargs.path, &path);
    ERROR_OUT_RET(ret);

    ret = do_mkdir(path);
    kfree(path);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_rmdir(argstr_t *args)
{
    argstr_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *path;
    ret = user_strdup(&kargs, &path);
    ERROR_OUT_RET(ret);

    ret = do_rmdir(path);
    kfree(path);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_unlink(argstr_t *args)
{
    argstr_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *path;
    ret = user_strdup(&kargs, &path);
    ERROR_OUT_RET(ret);

    ret = do_unlink(path);
    kfree(path);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_link(link_args_t *args)
{
    link_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *to, *from;
    ret = user_strdup(&kargs.to, &to);
    ERROR_OUT_RET(ret);

    ret = user_strdup(&kargs.from, &from);
    if (ret)
    {
        kfree(to);
        ERROR_OUT_RET(ret);
    }

    ret = do_link(from, to);
    kfree(to);
    kfree(from);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_rename(rename_args_t *args)
{
    rename_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *oldpath, *newpath;
    ret = user_strdup(&kargs.oldpath, &oldpath);
    ERROR_OUT_RET(ret);

    ret = user_strdup(&kargs.newpath, &newpath);
    if (ret)
    {
        kfree(oldpath);
        ERROR_OUT_RET(ret);
    }

    ret = do_rename(oldpath, newpath);
    kfree(oldpath);
    kfree(newpath);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_chdir(argstr_t *args)
{
    argstr_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *path;
    ret = user_strdup(&kargs, &path);
    ERROR_OUT_RET(ret);

    ret = do_chdir(path);
    kfree(path);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_lseek(lseek_args_t *args)
{
    lseek_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    ret = do_lseek(kargs.fd, kargs.offset, kargs.whence);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_open(open_args_t *args)
{
    open_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *path;
    ret = user_strdup(&kargs.filename, &path);
    ERROR_OUT_RET(ret);

    ret = do_open(path, kargs.flags);
    kfree(path);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_munmap(munmap_args_t *args)
{
    munmap_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    ret = do_munmap(kargs.addr, kargs.len);

    ERROR_OUT_RET(ret);
    return ret;
}

static void *sys_mmap(mmap_args_t *arg)
{
    mmap_args_t kargs;

    if (copy_from_user(&kargs, arg, sizeof(mmap_args_t)))
    {
        curthr->kt_errno = EFAULT;
        return MAP_FAILED;
    }

    void *ret;
    long err = do_mmap(kargs.mma_addr, kargs.mma_len, kargs.mma_prot,
                       kargs.mma_flags, kargs.mma_fd, kargs.mma_off, &ret);
    if (err)
    {
        curthr->kt_errno = -err;
        return MAP_FAILED;
    }
    return ret;
}

static pid_t sys_waitpid(waitpid_args_t *args)
{
    waitpid_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    int status;
    pid_t pid = do_waitpid(kargs.wpa_pid, &status, kargs.wpa_options);
    ERROR_OUT_RET(pid);

    if (kargs.wpa_status)
    {
        ret = copy_to_user(kargs.wpa_status, &status, sizeof(int));
        ERROR_OUT_RET(ret);
    }

    return pid;
}

static void *sys_brk(void *addr)
{
    void *new_brk;
    long ret = do_brk(addr, &new_brk);
    if (ret)
    {
        curthr->kt_errno = -ret;
        return (void *)-1;
    }
    return new_brk;
}

static void sys_halt(void) { proc_kill_all(); }

static long sys_stat(stat_args_t *args)
{
    stat_args_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *path;
    ret = user_strdup(&kargs.path, &path);
    ERROR_OUT_RET(ret);

    stat_t stat_buf;
    ret = do_stat(path, &stat_buf);
    kfree(path);
    ERROR_OUT_RET(ret);

    ret = copy_to_user(kargs.buf, &stat_buf, sizeof(stat_buf));
    ERROR_OUT_RET(ret);

    return ret;
}

static long sys_pipe(int args[2])
{
    int kargs[2];
    long ret = do_pipe(kargs);
    ERROR_OUT_RET(ret);

    ret = copy_to_user(args, kargs, sizeof(kargs));
    ERROR_OUT_RET(ret);

    return ret;
}

static long sys_uname(struct utsname *arg)
{
    static const char sysname[] = "Weenix";
    static const char release[] = "1.2";
    /* Version = last compilation time */
    static const char version[] = "#1 " __DATE__ " " __TIME__;
    static const char nodename[] = "";
    static const char machine[] = "";
    long ret = 0;

    ret = copy_to_user(arg->sysname, sysname, sizeof(sysname));
    ERROR_OUT_RET(ret);
    ret = copy_to_user(arg->release, release, sizeof(release));
    ERROR_OUT_RET(ret);
    ret = copy_to_user(arg->version, version, sizeof(version));
    ERROR_OUT_RET(ret);
    ret = copy_to_user(arg->nodename, nodename, sizeof(nodename));
    ERROR_OUT_RET(ret);
    ret = copy_to_user(arg->machine, machine, sizeof(machine));
    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_time(time_t *utloc)
{
    time_t time = do_time();
    if (utloc)
    {
        long ret = copy_to_user(utloc, &time, sizeof(time_t));
        ERROR_OUT_RET(ret);
    }
    return time;
}

static long sys_fork(regs_t *regs)
{
    long ret = do_fork(regs);
    ERROR_OUT_RET(ret);
    return ret;
}

static void free_vector(char **vect)
{
    char **temp;
    for (temp = vect; *temp; temp++)
    {
        kfree(*temp);
    }
    kfree(vect);
}

static long sys_execve(execve_args_t *args, regs_t *regs)
{
    execve_args_t kargs;
    char *filename = NULL;
    char **argv = NULL;
    char **envp = NULL;

    long ret;
    if ((ret = copy_from_user(&kargs, args, sizeof(kargs))))
        goto cleanup;

    if ((ret = user_strdup(&kargs.filename, &filename)))
        goto cleanup;

    if (kargs.argv.av_vec && (ret = user_vecdup(&kargs.argv, &argv)))
        goto cleanup;

    if (kargs.envp.av_vec && (ret = user_vecdup(&kargs.envp, &envp)))
        goto cleanup;

    ret = do_execve(filename, argv, envp, regs);

cleanup:
    if (filename)
        kfree(filename);
    if (argv)
        free_vector(argv);
    if (envp)
        free_vector(envp);
    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_debug(argstr_t *args)
{
    argstr_t kargs;
    long ret = copy_from_user(&kargs, args, sizeof(kargs));
    ERROR_OUT_RET(ret);

    char *str;
    ret = user_strdup(&kargs, &str);
    ERROR_OUT_RET(ret);
    dbg(DBG_USER, "%s\n", str);
    kfree(str);
    return ret;
}

static long sys_kshell(int ttyid)
{
    // ignoring the ttyid passed in as it always defaults to 0,
    // instead using the active_tty value
    kshell_t *ksh = kshell_create(active_tty);
    ERROR_OUT(!ksh, ENODEV);

    long ret;
    while ((ret = kshell_execute_next(ksh)) > 0)
        ;
    kshell_destroy(ksh);

    ERROR_OUT_RET(ret);
    return ret;
}

static long sys_usleep(usleep_args_t *args)
{
    return do_usleep(args->usec);
}

static inline void check_curthr_cancelled()
{
    KASSERT(list_empty(&curthr->kt_mutexes));
    spinlock_lock(&curthr->kt_lock);
    long cancelled = curthr->kt_cancelled;
    void *retval = curthr->kt_retval;
    spinlock_unlock(&curthr->kt_lock);

    if (cancelled)
    {
        dbg(DBG_SYSCALL, "CANCELLING: thread 0x%p of P%d (%s)\n", curthr,
            curproc->p_pid, curproc->p_name);
        kthread_exit(retval);
    }
}

static long syscall_handler(regs_t *regs)
{
    size_t sysnum = (size_t)regs->r_rax;
    uintptr_t args = (uintptr_t)regs->r_rdx;

    const char *syscall_string;
    if (sysnum <= 47)
    {
        syscall_string = syscall_strings[sysnum];
    }
    else
    {
        if (sysnum == 9001)
        {
            syscall_string = "debug";
        }
        else if (sysnum == 9002)
        {
            syscall_string = "kshell";
        }
        else
        {
            syscall_string = "unknown";
        }
    }

    if (sysnum != SYS_errno)
        dbg(DBG_SYSCALL, ">> pid %d, sysnum: %lu (%s), arg: %lu (0x%p)\n",
            curproc->p_pid, sysnum, syscall_string, args, (void *)args);

    check_curthr_cancelled();
    long ret = syscall_dispatch(sysnum, args, regs);
    check_curthr_cancelled();

    if (sysnum != SYS_errno)
        dbg(DBG_SYSCALL, "<< pid %d, sysnum: %lu (%s), returned: %lu (%#lx)\n",
            curproc->p_pid, sysnum, syscall_string, ret, ret);

    regs->r_rax = (uint64_t)ret;
    return 0;
}

static long syscall_dispatch(size_t sysnum, uintptr_t args, regs_t *regs)
{
    switch (sysnum)
    {
    case SYS_waitpid:
        return sys_waitpid((waitpid_args_t *)args);

    case SYS_exit:
        do_exit((int)args);
        panic("exit failed!\n");

    case SYS_thr_exit:
        kthread_exit((void *)args);
        panic("thr_exit failed!\n");

    case SYS_sched_yield:
        sched_yield();
        return 0;

    case SYS_fork:
        return sys_fork(regs);

    case SYS_getpid:
        return curproc->p_pid;

    case SYS_sync:
        do_sync();
        return 0;

#ifdef __MOUNTING__
    case SYS_mount:
        return sys_mount((mount_args_t *)args);

    case SYS_umount:
        return sys_umount((argstr_t *)args);
#endif

    case SYS_mmap:
        return (long)sys_mmap((mmap_args_t *)args);

    case SYS_munmap:
        return sys_munmap((munmap_args_t *)args);

    case SYS_open:
        return sys_open((open_args_t *)args);

    case SYS_close:
        return sys_close((int)args);

    case SYS_read:
        return sys_read((read_args_t *)args);

    case SYS_write:
        return sys_write((write_args_t *)args);

    case SYS_dup:
        return sys_dup((int)args);

    case SYS_dup2:
        return sys_dup2((dup2_args_t *)args);

    case SYS_mkdir:
        return sys_mkdir((mkdir_args_t *)args);

    case SYS_rmdir:
        return sys_rmdir((argstr_t *)args);

    case SYS_unlink:
        return sys_unlink((argstr_t *)args);

    case SYS_link:
        return sys_link((link_args_t *)args);

    case SYS_rename:
        return sys_rename((rename_args_t *)args);

    case SYS_chdir:
        return sys_chdir((argstr_t *)args);

    case SYS_getdents:
        return sys_getdents((getdents_args_t *)args);

    case SYS_brk:
        return (long)sys_brk((void *)args);

    case SYS_lseek:
        return sys_lseek((lseek_args_t *)args);

    case SYS_halt:
        sys_halt();
        return -1;

    case SYS_set_errno:
        curthr->kt_errno = (long)args;
        return 0;

    case SYS_errno:
        return curthr->kt_errno;

    case SYS_execve:
        return sys_execve((execve_args_t *)args, regs);

    case SYS_stat:
        return sys_stat((stat_args_t *)args);

    case SYS_pipe:
        return sys_pipe((int *)args);

    case SYS_uname:
        return sys_uname((struct utsname *)args);

    case SYS_time:
        return sys_time((time_t *)args);

    case SYS_debug:
        return sys_debug((argstr_t *)args);

    case SYS_kshell:
        return sys_kshell((int)args);

    case SYS_usleep:
        return sys_usleep((usleep_args_t *)args);

    default:
        dbg(DBG_ERROR, "ERROR: unknown system call: %lu (args: 0x%p)\n",
            sysnum, (void *)args);
        curthr->kt_errno = ENOSYS;
        return -1;
    }
}
