/*
 *  unistd.h - Standard Weenix System Interface
 */
#pragma once

#include "lseek.h"
#include "stdarg.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "weenix/config.h"

#ifndef NULL
#define NULL 0
#endif

struct dirent;

/* User exec-related */
int fork(void);

int execl(const char *filename, const char *arg, ...);  /* NYI */
int execle(const char *filename, const char *arg, ...); /* NYI */
int execv(const char *filename, char *const argv[]);    /* NYI */
int execve(const char *filename, char *const argv[], char *const envp[]);

/* Kern-related */
pid_t wait(int *status);

pid_t waitpid(pid_t pid, int *status, int options);

void thr_exit(int status);

int thr_errno(void);

void thr_set_errno(int n);

int sched_yield(void);

pid_t getpid(void);

int halt(void);

void sync(void);

size_t get_free_mem(void);

/* VFS-related */
int open(const char *filename, int flags, int mode);

int close(int fd);

ssize_t read(int fd, void *buf, size_t count);

ssize_t write(int fd, const void *buf, size_t count);

off_t lseek(int fd, off_t offset, int whence);

int dup(int fd);

int dup2(int ofd, int nfd);

int mkdir(const char *path, int mode);

int rmdir(const char *path);

int unlink(const char *path);

int link(const char *oldpath, const char *newpath);

int rename(const char *oldpath, const char *newpath);

int chdir(const char *path);

int getdents(int fd, struct dirent *dir, size_t size);

int stat(const char *path, struct stat *buf);

int pipe(int pipefd[2]);

/* VM-related */
void *mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off);

int munmap(void *addr, size_t len);

int brk(void *addr);

void *sbrk(intptr_t incr);

/* Mounting */
int mount(const char *source, const char *target, const char *filesystemtype,
          unsigned long mountflags, const void *data);

int umount(const char *target);

time_t time(time_t *tloc);

long usleep(useconds_t usec);

#define STDIN_FILENO 0
#define STDOUT_FILENO 1
#define STDERR_FILENO 2
