// SMP.1 + SMP.3
// spinlock + mask interrupts
#include "config.h"
#include "errno.h"
#include "fs/file.h"
#include "fs/vfs.h"
#include "fs/vnode.h"
#include "globals.h"
#include "kernel.h"
#include "mm/slab.h"
#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"
#include "util/time.h"
#include <drivers/screen.h>
#include <fs/vfs_syscall.h>
#include <main/apic.h>

/*==========
 * Variables
 *=========*/

/*
 * Global variable that maintains the current process
 */
proc_t *curproc CORE_SPECIFIC_DATA;

/*
 * Global list of all processes (except for the idle process) and its lock
 */
static list_t proc_list = LIST_INITIALIZER(proc_list);
static spinlock_t proc_list_lock = SPINLOCK_INITIALIZER(proc_list_lock);

/*
 * Allocator for process descriptors
 */
static slab_allocator_t *proc_allocator = NULL;

/*
 * Statically allocated idle process
 * Each core has its own idleproc, so the idleproc is stored in static memory
 * rather than in the global process list
 */
proc_t idleproc CORE_SPECIFIC_DATA;

/*
 * Pointer to the init process
 */
static proc_t *proc_initproc = NULL;

/*===============
 * System startup
 *==============*/

/*
 * Initializes the allocator for process descriptors.
 */
void proc_init()
{
    proc_allocator = slab_allocator_create("proc", sizeof(proc_t));
    KASSERT(proc_allocator);
}

/*
 * Initializes idleproc for the current core. Sets initial values for curproc
 * and curthr.
 */
void proc_idleproc_init()
{
    proc_t *proc = &idleproc;

    proc->p_pid = 0;
    list_init(&proc->p_threads);
    list_init(&proc->p_children);
    proc->p_pproc = NULL;

    list_link_init(&proc->p_child_link);
    list_link_init(&proc->p_list_link);

    spinlock_init(&proc->p_children_lock);

    proc->p_status = 0;
    proc->p_state = PROC_RUNNING;

    memset(&proc->p_wait, 0, sizeof(ktqueue_t)); // should not be used

    proc->p_pml4 = pt_get();
    #ifdef __VM__
        proc->p_vmmap = vmmap_create();
    #endif  

    proc->p_cwd = NULL;

    memset(proc->p_files, 0, sizeof(proc->p_files));

    char name[8];
    snprintf(name, sizeof(name), "idle%ld", curcore.kc_id);
    strncpy(proc->p_name, name, PROC_NAME_LEN);
    proc->p_name[PROC_NAME_LEN - 1] = '\0';

    dbg(DBG_PROC, "created %s\n", proc->p_name);
    curproc = &idleproc;
    curthr = NULL;
}

/*=================
 * Helper functions
 *================*/

/*
 * Gets the next available process ID (pid).
 */
static pid_t next_pid = 1;
static pid_t _proc_getid()
{
    spinlock_lock(&proc_list_lock);
    pid_t pid = next_pid;
restart:
    list_iterate(&proc_list, p, proc_t, p_list_link)
    {
        if (p->p_pid == pid)
        {
            pid = pid + 1 == PROC_MAX_COUNT ? 1 : pid + 1;
            if (pid == next_pid)
            {
                spinlock_unlock(&proc_list_lock);
                return -1;
            }
            else
            {
                goto restart;
            }
        }
    }
    next_pid = pid + 1 == PROC_MAX_COUNT ? 1 : pid + 1;
    KASSERT(pid);
    spinlock_unlock(&proc_list_lock);
    return pid;
}

/*
 * Searches the global process list for the process descriptor corresponding to
 * a pid.
 */
proc_t *proc_lookup(pid_t pid)
{
    if (pid == 0)
    {
        return &idleproc;
    }
    spinlock_lock(&proc_list_lock);
    list_iterate(&proc_list, p, proc_t, p_list_link)
    {
        if (p->p_pid == pid)
        {
            spinlock_unlock(&proc_list_lock);
            return p;
        }
    }
    spinlock_unlock(&proc_list_lock);
    return NULL;
}

/*==========
 * Functions
 *=========*/

/*
 * Creates a new process with the given name.
 * Returns the newly created process, or null on failure.
 *
 * Hints:
 * Use _proc_getid() to get a new pid.
 * Allocate a new proc_t with the process slab allocator (proc_allocator).
 * Use pt_create() to create a new page table (p_pml4).
 * If the newly created process is the init process (i.e. the generated PID 
 * matches the init process's PID, given by the macro PID_INIT), set the 
 * global proc_initproc to the created process.
 * 
 * There is some setup to be done for VFS and VM - remember to return to this
 * function! For VFS, clone and ref the files from curproc. For VM, clone the
 * vmmap from curproc. 
 * 
 * Be sure to free resources appropriately if proc_create() fails midway!
 */
proc_t *proc_create(const char *name)
{
    if (strlen(name) > PROC_NAME_LEN) {
        return NULL;
    }
    pid_t pid = _proc_getid();
    proc_t* proc = (proc_t*) slab_obj_alloc(proc_allocator);
    if (proc == NULL) {
        return NULL;
    }
    pml4_t* page = pt_create();
    if (pid == PID_INIT) {
        proc_initproc = proc;
    }
    proc->p_pid = pid;
    memset(proc->p_name, 0, PROC_NAME_LEN);
    for (size_t i = 0; i < strlen(name); i++) {
        proc->p_name[i] = name[i];
    }
    proc->p_pml4 = page;
    proc->p_state = PROC_RUNNING;
    proc->p_pproc = curproc;
    list_init(&proc->p_children);
    list_init(&proc->p_threads);
    list_link_init(&proc->p_list_link);
    list_link_init(&proc->p_child_link);
    sched_queue_init(&proc->p_wait);
    list_insert_head(&curproc->p_children, &proc->p_child_link);
    sched_queue_init(&proc->p_wait);
    vmmap_t* map_clone = vmmap_clone(curproc->p_vmmap);
    map_clone->vmm_proc = proc;
    proc->p_vmmap = map_clone;
    spinlock_lock(&proc_list_lock);
    list_insert_head(&proc_list, &proc->p_list_link);
    proc->p_cwd = curproc->p_cwd;
    proc->p_brk = curproc->p_brk;
    proc->p_start_brk = curproc->p_start_brk;
    if (proc->p_cwd) {
        vref(curproc->p_cwd);
    }
    for (int i = 0; i < NFILES; i++) {
        proc->p_files[i] = curproc->p_files[i];
        if (proc->p_files[i]) {
            fref(proc->p_files[i]);
        }
    }
    spinlock_unlock(&proc_list_lock);
    return proc;
}

/*
 * Helper for proc_thread_exiting() that cleans up resources from the current
 * process in preparation for its destruction (which occurs later via proc_destroy()). 
 * Reparents child processes to the init process, or initiates Weenix shutdown 
 * if the current process is the init process.
 *
 * Hints:
 * You won't have much to clean up until VFS and VM -- remember to revisit this
 * function later!
 * **VFS/VM** - there may be some repeat code in proc_destroy()). The initial process
 * does not have a parent process and thus cleans itself up, hence why we need to cleanup 
 * here as well. 
 * 
 * Remember to set the state and status of the process.
 * The init process' PID is given by PID_INIT
 * Use initproc_finish() to shutdown Weenix when cleaning up the init process.
 */
void proc_cleanup(long status)
{
    curproc->p_state = PROC_DEAD;
    for (int fd = 0; fd < NFILES; fd++)
    {
        do_close(fd);
    }
    vput(&curproc->p_cwd);
    vmmap_destroy(&curproc->p_vmmap);
    if (curproc->p_pid == PID_INIT) {
        initproc_finish();
    } else {
        list_iterate(&curproc->p_children, child, proc_t, p_child_link) {
            list_remove(&child->p_child_link);
            list_insert_head(&proc_initproc->p_children, &child->p_child_link);
            child->p_pproc = proc_initproc;
        }
    }
}

/*
 * Cleans up the current process and the current thread, broadcasts on its
 * parent's p_wait, then forces a context switch. After this, the process is
 * essentially dead -- this function does not return. The parent must eventually
 * finish destroying the process.
 *
 * Hints:
 * Use proc_cleanup() to clean up the current process (you should pass (long)retval
 * as the status argument).
 * Remember to set the exit state and return value of the current thread after calling
 * proc_cleanup(), as this may block and cause the thread's state to be overwritten. 
 * The context switch should be performed by a call to sched_switch().
 */
void proc_thread_exiting(void *retval)
{
    proc_cleanup((long)retval);
    //curthr->kt_retval = retval;
    curthr->kt_state = KT_EXITED;
    sched_broadcast_on(&curproc->p_pproc->p_wait);
    sched_switch(0, 0);
}

/*
 * Cancels all the threads of proc. This should never be called on curproc.
 * 
 * Hints:
 * The status argument should be passed to kthread_cancel() as the retval.
 */
void proc_kill(proc_t *proc, long status)
{
    KASSERT(proc != curproc);
    list_iterate(&proc->p_threads, thread, kthread_t, kt_plink) {
        kthread_cancel(thread, &status);
    }
}

/*
 * Kills all processes that are not curproc and not a direct child of idleproc (i.e.,
 * the init process), then kills the current process.
 *
 * Hints:
 * The PID of the idle process is given by PID_IDLE.
 * Processes should be killed with a status of -1.
 * Use do_exit() to kill the current process.
 */
void proc_kill_all()
{
    spinlock_lock(&proc_list_lock);
    list_iterate(&proc_list, p, proc_t, p_list_link) {
        if (p->p_pid != curproc->p_pid && curproc->p_pproc != PID_IDLE) {
            proc_kill(p, -1);
        }
    }
    spinlock_unlock(&proc_list_lock);
    do_exit(-1);
}

/*
 * Destroy / free everything from proc. Be sure to remember reference counting
 * when working on VFS.
 *
 * In contrast with proc_cleanup() (in which a process begins to clean itself up), this 
 * will be called on proc by some other process to complete its cleanup. 
 * I.e., the process we are destroying should not be curproc.
 */
void proc_destroy(proc_t *proc)
{
    spinlock_lock(&proc_list_lock);
    list_remove(&proc->p_list_link);
    spinlock_unlock(&proc_list_lock);

    list_iterate(&proc->p_threads, thr, kthread_t, kt_plink)
    {
        kthread_destroy(thr);
    }

#ifdef __VFS__
    for (int fd = 0; fd < NFILES; fd++)
    {
        if (proc->p_files[fd])
            fput(proc->p_files + fd);
    }
    if (proc->p_cwd)
    {
        vput(&proc->p_cwd);
    }
#endif

#ifdef __VM__
    if (proc->p_vmmap)
        vmmap_destroy(&proc->p_vmmap);
#endif

    dbg(DBG_THR, "destroying P%d\n", proc->p_pid);

    KASSERT(proc->p_pml4);
    pt_destroy(proc->p_pml4);

    slab_obj_free(proc_allocator, proc);
}

/*=============
 * System calls
 *============*/

/*
 * Waits for a child process identified by pid to exit. Finishes destroying the
 * process and optionally returns the child's status in status.
 *
 * If pid is a positive integer, tries to clean up the process specified by pid.
 * If pid is -1, cleans up any child process of curproc that exits.
 *
 * Returns the pid of the child process that exited, or error cases:
 *  - ENOTSUP: pid is 0, a negative number not equal to -1,
 *      or options are specified (options does not equal 0)
 *  - ECHILD: pid is a positive integer but not a child of curproc, or
 *      pid is -1 and the process has no children
 *
 * Hints:
 * Use sched_sleep_on() to be notified of a child process exiting.
 * Destroy an exited process by removing it from any lists and calling
 * proc_destroy(). Remember to set status (if it was provided) to the child's
 * status before destroying the process.
 * If waiting on a specific child PID, wakeups from other exiting child
 * processes should be ignored.
 * If waiting on any child (-1), do_waitpid can return when *any* child has exited,
 * it does not have to return the one that exited earliest.
 */
pid_t do_waitpid(pid_t pid, int *status, int options) {
    if (pid == 0 || pid < -1 || options) {
        return -ENOTSUP;
    }
    spinlock_lock(&curproc->p_children_lock);
    if (pid == -1 && list_empty(&curproc->p_children)) {
        spinlock_unlock(&curproc->p_children_lock);
        return -ECHILD;
    }
    if (pid > 0) {
        list_iterate(&curproc->p_children, ch, proc_t, p_child_link) {
            if (ch->p_pid == pid) {
                if (ch->p_state == PROC_DEAD) {
                    *status = ch->p_status;
                    proc_destroy(ch);
                } else {
                    while (ch->p_state != PROC_DEAD) {
                        sched_sleep_on(&curproc->p_wait, &curproc->p_children_lock);
                    }
                    *status = ch->p_status;
                    pid_t child_pid = ch->p_pid;
                    list_remove(&ch->p_child_link);
                    proc_destroy(ch);
                    return child_pid;
                }
            }
        }
        return -ECHILD;
    } else if (pid == -1) {
        while (1) {
            list_iterate(&curproc->p_children, child, proc_t, p_child_link) {
                if (child->p_state == PROC_DEAD) {
                    *status = child->p_status;
                    pid_t child_pid = child->p_pid;
                    list_remove(&child->p_child_link);
                    proc_destroy(child);
                    return child_pid;
                }
            }
            sched_sleep_on(&curproc->p_wait, &curproc->p_children_lock);
        }
    }
    return -ENOTSUP;
}

/*
 * Wrapper around kthread_exit.
 */
void do_exit(long status)
{
    // keep this line I think
    curproc->p_status = status;
    kthread_exit(&status);
}

/*==========
 * Debugging
 *=========*/

size_t proc_info(const void *arg, char *buf, size_t osize)
{
    const proc_t *p = (proc_t *)arg;
    size_t size = osize;
    proc_t *child;

    KASSERT(NULL != p);
    KASSERT(NULL != buf);

    iprintf(&buf, &size, "pid:          %i\n", p->p_pid);
    iprintf(&buf, &size, "name:         %s\n", p->p_name);
    if (NULL != p->p_pproc)
    {
        iprintf(&buf, &size, "parent:       %i (%s)\n", p->p_pproc->p_pid,
                p->p_pproc->p_name);
    }
    else
    {
        iprintf(&buf, &size, "parent:       -\n");
    }

    if (list_empty(&p->p_children))
    {
        iprintf(&buf, &size, "children:     -\n");
    }
    else
    {
        iprintf(&buf, &size, "children:\n");
    }
    list_iterate(&p->p_children, child, proc_t, p_child_link)
    {
        iprintf(&buf, &size, "     %i (%s)\n", child->p_pid, child->p_name);
    }

    iprintf(&buf, &size, "status:       %ld\n", p->p_status);
    iprintf(&buf, &size, "state:        %i\n", p->p_state);

#ifdef __VFS__
#ifdef __GETCWD__
    if (NULL != p->p_cwd)
    {
        char cwd[256];
        lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
        iprintf(&buf, &size, "cwd:          %-s\n", cwd);
    }
    else
    {
        iprintf(&buf, &size, "cwd:          -\n");
    }
#endif /* __GETCWD__ */
#endif

#ifdef __VM__
    iprintf(&buf, &size, "start brk:    0x%p\n", p->p_start_brk);
    iprintf(&buf, &size, "brk:          0x%p\n", p->p_brk);
#endif

    return size;
}

size_t proc_list_info(const void *arg, char *buf, size_t osize)
{
    size_t size = osize;

    KASSERT(NULL == arg);
    KASSERT(NULL != buf);

#if defined(__VFS__) && defined(__GETCWD__)
    iprintf(&buf, &size, "%5s %-13s %-18s %-s\n", "PID", "NAME", "PARENT",
            "CWD");
#else
    iprintf(&buf, &size, "%5s %-13s %-s\n", "PID", "NAME", "PARENT");
#endif

    list_iterate(&proc_list, p, proc_t, p_list_link)
    {
        char parent[64];
        if (NULL != p->p_pproc)
        {
            snprintf(parent, sizeof(parent), "%3i (%s)", p->p_pproc->p_pid,
                     p->p_pproc->p_name);
        }
        else
        {
            snprintf(parent, sizeof(parent), "  -");
        }

#if defined(__VFS__) && defined(__GETCWD__)
        if (NULL != p->p_cwd)
        {
            char cwd[256];
            lookup_dirpath(p->p_cwd, cwd, sizeof(cwd));
            iprintf(&buf, &size, " %3i  %-13s %-18s %-s\n", p->p_pid, p->p_name,
                    parent, cwd);
        }
        else
        {
            iprintf(&buf, &size, " %3i  %-13s %-18s -\n", p->p_pid, p->p_name,
                    parent);
        }
#else
        iprintf(&buf, &size, " %3i  %-13s %-s\n", p->p_pid, p->p_name, parent);
#endif
    }
    return size;
}
