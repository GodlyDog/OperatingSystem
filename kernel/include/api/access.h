#pragma once

#include "types.h"

struct proc;
struct argstr;
struct argvec;

long copy_from_user(void *kaddr, const void *uaddr, size_t nbytes);

long copy_to_user(void *uaddr, const void *kaddr, size_t nbytes);

long user_strdup(struct argstr *ustr, char **kstrp);

long user_vecdup(struct argvec *uvec, char ***kvecp);

long range_perm(struct proc *p, const void *vaddr, size_t len, int perm);

long addr_perm(struct proc *p, const void *vaddr, int perm);
