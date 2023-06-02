#include <errno.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[], char *envp[])
{
    printf("pid %d: Entering forktest\n", getpid());
    int pid = fork();
    printf("pid %d: Fork returned %d\n", getpid(), pid);
    printf("pid %d: About to enter waitpid\n", getpid());
    int rc = waitpid(-1, 0, 0);
    printf("pid %d: Waitpid returned %d\n", getpid(), rc);
    if (rc == -1)
    {
        printf("pid %d: ERRNO = %d\n", getpid(), errno);
    }
    printf("pid %d: Exiting\n", getpid());
    return 0;
}
