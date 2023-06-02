#pragma once

#include "types.h"

struct regs;

long do_execve(const char *filename, char *const *argv, char *const *envp,
               struct regs *regs);

void kernel_execve(const char *filename, char *const *argv, char *const *envp);

void userland_entry(struct regs regs);
