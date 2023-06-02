#include "types.h"

struct proc;

long do_munmap(void *addr, size_t len);

long do_mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off,
             void **ret);
