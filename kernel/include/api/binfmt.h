#pragma once

#include "fs/vnode.h"

typedef long (*binfmt_load_func_t)(const char *filename, int fd,
                                   char *const *argv, char *const *envp,
                                   uint64_t *rip, uint64_t *rsp);

long binfmt_add(const char *id, binfmt_load_func_t loadfunc);

long binfmt_load(const char *filename, char *const *argv, char *const *envp,
                 uint64_t *rip, uint64_t *rsp);
