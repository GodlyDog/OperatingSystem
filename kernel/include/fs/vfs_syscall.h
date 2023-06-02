#pragma once

#include "dirent.h"

#include "types.h"

#include "fs/open.h"
#include "fs/pipe.h"
#include "fs/stat.h"

long do_close(int fd);

ssize_t do_read(int fd, void *buf, size_t len);

ssize_t do_write(int fd, const void *buf, size_t len);

long do_dup(int fd);

long do_dup2(int ofd, int nfd);

long do_mknod(const char *path, int mode, devid_t devid);

long do_mkdir(const char *path);

long do_rmdir(const char *path);

long do_unlink(const char *path);

long do_link(const char *oldpath, const char *newpath);

long do_rename(const char *oldpath, const char *newpath);

long do_chdir(const char *path);

ssize_t do_getdent(int fd, struct dirent *dirp);

off_t do_lseek(int fd, off_t offset, int whence);

long do_stat(const char *path, struct stat *uf);
