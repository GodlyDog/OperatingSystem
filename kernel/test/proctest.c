#include "errno.h"
#include "globals.h"

#include "test/proctest.h"
#include "test/usertest.h"

#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"

#include "proc/kthread.h"
#include "proc/proc.h"
#include "proc/sched.h"

/*
 * Set up a testing function for the process to execute. 
*/
void *test_func(long arg1, void *arg2)
{
    proc_t *proc_as_arg = (proc_t *)arg2;
    test_assert(arg1 == proc_as_arg->p_pid, "Arguments are not set up correctly");
    test_assert(proc_as_arg->p_state == PROC_RUNNING, "Process state is not running");
    test_assert(list_empty(&proc_as_arg->p_children), "There should be no child processes");
    sched_yield();
    return NULL;
}

void *spin_func(long arg1, void *arg2) {
    ktqueue_t *queue_as_arg = (ktqueue_t *)arg2;
    proc_t* this_proc = proc_lookup(arg1);
    kthread_t* this_thread = list_head(&this_proc->p_threads, kthread_t, kt_plink);
    sched_cancellable_sleep_on(queue_as_arg, NULL);
    dbg(DBG_TEST, "\nReturned sleep\n");
    test_assert(this_thread->kt_cancelled == 1, "Thread is not cancelled correctly");
    kthread_exit(NULL);
    return NULL;
}

void *spin_no_cancel(long arg1, void *arg2) {
    ktqueue_t *queue_as_arg = (ktqueue_t *)arg2;
    proc_t* this_proc = proc_lookup(arg1);
    kthread_t* this_thread = list_head(&this_proc->p_threads, kthread_t, kt_plink);
    sched_cancellable_sleep_on(queue_as_arg, NULL);
    dbg(DBG_TEST, "\nReturned sleep\n");
    test_assert(this_thread->kt_cancelled != 1, "Thread is cancelled");
    return NULL;
}

void test_termination()
{
    int num_procs_created = 0;
    for (int i = 0; i < 10; i++) {
        proc_t *new_proc1 = proc_create("proc test 1");
        kthread_t *new_kthread1 = kthread_create(new_proc1, test_func, new_proc1->p_pid, new_proc1);
        num_procs_created++;
        sched_make_runnable(new_kthread1);
    }
    int count = 0;
    int status;
    while (do_waitpid(-1, &status, 0) != -ECHILD) {
        test_assert(status == 0, "Returned status not set correctly");
        count++;
    }
    test_assert(count == num_procs_created,
                "Expected: %d, Actual: %d number of processes have been cleaned up\n", num_procs_created, count);
}

void test_waitpid() {
    int num_procs_created = 0;
    proc_t *new_proc1 = proc_create("proc test 1");
    kthread_t *new_kthread1 = kthread_create(new_proc1, test_func, new_proc1->p_pid, new_proc1);
    num_procs_created++;
    sched_make_runnable(new_kthread1);

    int count = 0;
    int status;
    pid_t pid = new_proc1 -> p_pid;
    while (do_waitpid(pid, &status, 0) != -ECHILD) {
        test_assert(status == 0, "Returned status not set correctly");
        count++;
    }
    test_assert(count == num_procs_created,
                "Expected: %d, Actual: %d number of processes have been cleaned up\n", num_procs_created, count);
}

void test_sleep() {
    int num_procs_created = 0;
    ktqueue_t queue;
    sched_queue_init(&queue);
    test_assert(sched_queue_empty(&queue), "Cancellation queue not set up correctly");
    proc_t *new_proc1 = proc_create("proc test 1");
    kthread_t *new_kthread1 = kthread_create(new_proc1, spin_func, new_proc1->p_pid, &queue);
    num_procs_created++;
    sched_make_runnable(new_kthread1);
    sched_yield();
    proc_kill(new_proc1, -1);
    int count = 0;
    int status;
    test_assert(sched_queue_empty(&queue), "Cancellation queue not empty");
    while (do_waitpid(-1, &status, 0) != -ECHILD) {
        test_assert(status == 0, "Returned status not set correctly");
        count++;
    }
    test_assert(count == num_procs_created,
                "Expected: %d, Actual: %d number of processes have been cleaned up\n", num_procs_created, count);
    test_assert(sched_queue_empty(&queue), "Cancellation queue not empty");
    proc_t *new_proc2 = proc_create("proc test 2");
    kthread_t *new_kthread2 = kthread_create(new_proc2, spin_no_cancel, new_proc2->p_pid, &queue);
    num_procs_created++;
    sched_make_runnable(new_kthread2);
    sched_yield();
    kthread_t* compare_thread = NULL;
    sched_wakeup_on(&queue, &compare_thread);
    test_assert(compare_thread == new_kthread2, "Returned thread pointer not set correctly");
    while (do_waitpid(-1, &status, 0) != -ECHILD) {
        test_assert(status == 0, "Returned status not set correctly");
        count++;
    }
    test_assert(count == num_procs_created,
                "Expected: %d, Actual: %d number of processes have been cleaned up\n", num_procs_created, count);
}

long proctest_main(long arg1, void *arg2)
{
    dbg(DBG_TEST, "\nStarting Procs tests\n");
    test_init();
    test_termination();
    test_waitpid();
    dbg(DBG_TEST, "\nStarting cancellation tests\n");
    test_sleep();
    dbg(DBG_TEST, "\nFinished Procs tests\n");
    test_fini();
    return 0;
}