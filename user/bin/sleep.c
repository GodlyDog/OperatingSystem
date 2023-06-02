#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SECONDS_TO_MICROSECONDS 1000000

void help(int argc, char *argv[])
{
    fprintf(stderr, "usage: %s [-u (micrseconds)] <time>\n", argv[0]);
    exit(1);
}

int main(int argc, char *argv[])
{
    if (argc > 3 || argc == 1)
    {
        help(argc, argv);
    }

    uint64_t tm = 0;

    int use_usec = 0;
    if (argc > 2)
    {
        if (strcmp(argv[1], "-u") == 0)
        {
            tm = atoi(argv[2]);
            use_usec = 1;
        }
        else
        {
            help(argc, argv);
        }
    }

    if (!use_usec)
    {
        tm = atoi(argv[1]);
        tm *= SECONDS_TO_MICROSECONDS;
    }

    return usleep(tm);
}