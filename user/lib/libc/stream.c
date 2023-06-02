#include "stdio.h"

static FILE stdstreams[3] = {
    {.fd = 0, .offset = 0},
    {.fd = 1, .offset = 0},
    {.fd = 2, .offset = 0},
};

FILE *stdin = &stdstreams[0];
FILE *stdout = &stdstreams[1];
FILE *stderr = &stdstreams[2];
