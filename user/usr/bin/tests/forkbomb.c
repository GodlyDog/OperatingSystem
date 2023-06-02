#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* TODO add options for different ways of forkbombing
   (kind of low priority but would be fun) */

int main(int argc, char **argv)
{
    int n = 1;
    pid_t pid;

    open("/dev/tty0", O_RDONLY, 0);
    open("/dev/tty0", O_WRONLY, 0);
    printf("Forking up a storm!\n");
    printf("If this runs for 10 minutes without crashing, then you ");
    printf("probably aren't \nleaking resources\n");

    if (fork())
    {
        for (;;)
        {
            printf("Fork number : %d\n", n);
            if ((pid = fork()))
            {
                if (pid == -1)
                {
                    printf("Fork %d failed. Forkbomb stopping.\n", n);
                    exit(1);
                }
                int status;
                sched_yield();
                wait(&status);
                if (status != 0)
                {
                    printf("Test failed. Child exit with status %d\n", status);
                    exit(1);
                }
            }
            else
            {
                int a = 0;
                sched_yield();
                exit(0);
            }
            n++;
        }
    }

#if 0
// Old forkbomb 
    if (!fork())
    {
        for (;;)
        {
            printf("I am fork number %d\n", n);
            if ((pid = fork()))
            {
                if (-1 != pid)
                {
                    exit(0);
                }
                else
                {
                    printf(
                        "%d-th fork failed. "
                        "forkbomb stopping.",
                        n);
                    exit(1);
                }
            }
            ++n;
        }
    }
    else
    {
        int status;
        while (wait(&status) > 0)
            ;
    }
#endif
    return 0;
}
