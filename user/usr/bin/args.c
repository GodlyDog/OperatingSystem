/*
 * Does some basic checks to make sure arguments are
 * being passed to userland programs correctly.
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv, char **envp)
{
    int i;
    char buf[100];

    open("/dev/tty0", O_RDONLY, 0);
    open("/dev/tty0", O_WRONLY, 0);

    snprintf(buf, sizeof(buf), "Arguments: (argc = %d, argv = %p)\n", argc,
             argv);
    write(1, buf, strlen(buf));
    for (i = 0; argv[i]; i++)
    {
        snprintf(buf, sizeof(buf), "  %d \"%s\"\n", i, argv[i]);
        write(1, buf, strlen(buf));
    }
    snprintf(buf, sizeof(buf), "Environment: (envp = %p)\n", envp);
    write(1, buf, strlen(buf));
    for (i = 0; envp[i]; i++)
    {
        snprintf(buf, sizeof(buf), "  %d \"%s\"\n", i, envp[i]);
        write(1, buf, strlen(buf));
    }

    return 0;
}
