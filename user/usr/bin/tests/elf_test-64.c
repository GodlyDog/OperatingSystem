#include <stdio.h>

typedef struct
{
    long int a_type; /* Entry type */
    union {
        long int a_val;      /* Integer value */
        void *a_ptr;         /* Pointer value */
        void (*a_fcn)(void); /* Function pointer value */
    } a_un;
} Elf64_auxv_t;

int main(int argc, char **argv, char **envp, Elf64_auxv_t *auxv)
{
    // print argument count
    printf("argc: %d\n", argc);

    // print argument vector
    int i;
    for (i = 0; argv[i] != 0; i++)
    {
        printf("argv[%d] (%p): %s\n", i, argv[i], argv[i]);
    }

    // print environment vector
    for (i = 0; envp[i] != 0; i++)
    {
        printf("envp[%d] (%p): %s\n", i, envp[i], envp[i]);
    }

    // print auxiliary vector
    for (i = 0; auxv[i].a_type != 0; i++)
    {
        printf("auxv[%d]: type %ld\n", i, auxv[i].a_type);
    }

    return 0;
}