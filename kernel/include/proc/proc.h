#pragma once

#include "config.h"
#include "mm/pagetable.h"
#include "proc/kthread.h"
#include "types.h"
#include "vm/vmmap.h"

/*===========
 * Structures
 *==========*/

/*
 * Process resource information
 */
#define PROC_MAX_COUNT 65536
#define PROC_NAME_LEN 256

/* Process states */
typedef enum
{
    PROC_RUNNING, /* Has running threads */
    PROC_DEAD     /* Exited, but not yet wait'ed */
} proc_state_t;

/* Process descriptor */
typedef struct proc
{
    pid_t p_pid;                /* Process ID */
    char p_name[PROC_NAME_LEN]; /* Process name */

    list_t p_threads;  /* Threads list */
    list_t p_children; /* Children list */
    spinlock_t p_children_lock;
    struct proc *p_pproc; /* Parent process */

    list_link_t p_list_link;  /* Link of list of all processes */
    list_link_t p_child_link; /* Link on parent's list of children */

    long p_status;        /* Exit status */
    proc_state_t p_state; /* Process state */

    pml4_t *p_pml4; /* Page table. */

    /*
     * If a parent is waiting on a child, the parent puts itself on its own
     * p_wait queue. When a child terminates, it broadcasts on its parent's
     * p_wait to wake it up.
     */
    ktqueue_t p_wait;

    /* VFS related */
    struct file *p_files[NFILES]; /* Open files */
    struct vnode *p_cwd;          /* Current working directory */

    /* VM related */
    /*
     * The current value of a process's break is maintained in the 'p_brk'.
     *
     * The 'p_brk' and 'p_start_brk' members of a proc_t struct are initialized
     * by the loader. 'p_start_brk' is subsequently never modified; it always
     * holds the initial value of the break.
     *
     * The loader sets 'p_start_brk' to be the end of the bss section (search
     * online for memory layout diagrams of a running process for more
     * details).
     * 
     * These are both addresses. 
     */
    void *p_brk;           /* Process break; see brk(2) */
    void *p_start_brk;     /* Initial value of process break */
    struct vmmap *p_vmmap; /* List of areas mapped into process's
                              user address space. */
} proc_t;

/*==========
 * Functions
 *=========*/

/**
 * Initializes the proc subsystem at system startup.
 */
void proc_init(void);

/**
 * Initializes the special idleproc at system startup.
 */
void proc_idleproc_init();

/**
 * Shuts down certain subsystems at system shutdown.
 */
void initproc_finish();

/**
 * Allocates and initializes a new process.
 *
 * @param name the name to give the newly created process
 * @return the newly created process
 */
proc_t *proc_create(const char *name);

/**
 * Frees all the resources associated with a process.
 *
 * @param proc process to destroy
 */
void proc_destroy(proc_t *proc);


proc_t *proc_lookup(pid_t pid);

/**
 * Handles exiting the current process.
 *
 * @param retval exit code for the thread and process
 */
void proc_thread_exiting(void *retval);

/**
 * Stops another process from running again by cancelling all its
 * threads.
 *
 * @param proc the process to kill
 * @param status the status the process should exit with
 */
void proc_kill(proc_t *proc, long status);

/**
 * Kills every process except for the idle process and direct children
 * of the idle process.
 */
void proc_kill_all(void);

/*========================
 * Functions: System calls
 *=======================*/

/**
 * Implements the _exit(2) system call.
 *
 * @param status the exit status of the process
 */
void do_exit(long status);

/**
 * Implements the waitpid(2) system call.
 *
 * @param pid the pid to wait on, or -1 to wait on any child
 * @param status used to return the exit status of the child
 * @param options only 0 is supported (no options)
 *
 * @return the pid of the child process which was cleaned up, or
 *  - ENOTSUP invalid input
 *  - ECHILD valid child could not be found
 */
pid_t do_waitpid(pid_t pid, int *status, int options);

/**
 * This function implements the fork(2) system call.
 *
 * @param regs the register state at the time of the system call
 */
struct regs;
long do_fork(struct regs *regs);

/*===========
 * Miscellany
 *==========*/

/*
 * Special PIDs reserved for specific processes
 */
#define PID_IDLE 0
#define PID_INIT 1

/*
 * Enable global use of idleproc
 */
extern proc_t idleproc;

/*=====================
 * Functions: Debugging
 *====================*/

/**
 * Provides detailed debug information about a given process.
 *
 * @param arg a pointer to the process
 * @param buf buffer to write to
 * @param osize size of the buffer
 * @return the remaining size of the buffer
 */
size_t proc_info(const void *arg, char *buf, size_t osize);

/**
 * Provides debug information overview of all processes.
 *
 * @param arg must be NULL
 * @param buf buffer to write to
 * @param osize size of the buffer
 * @return the remaining size of the buffer
 */
size_t proc_list_info(const void *arg, char *buf, size_t osize);