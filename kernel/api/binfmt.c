#include "errno.h"

#include "main/inits.h"

#include "fs/fcntl.h"
#include "fs/file.h"
#include "fs/vfs_syscall.h"

#include "util/debug.h"
#include "util/init.h"
#include "util/list.h"

#include "mm/kmalloc.h"

#include "api/binfmt.h"

typedef struct binfmt
{
    const char *bf_id;
    binfmt_load_func_t bf_load;
    list_link_t bf_link;
} binfmt_t;

static list_t binfmt_list = LIST_INITIALIZER(binfmt_list);

long binfmt_add(const char *id, binfmt_load_func_t loadfunc)
{
    binfmt_t *fmt;
    if (NULL == (fmt = kmalloc(sizeof(*fmt))))
    {
        return -ENOMEM;
    }

    dbg(DBG_EXEC, "Registering binary loader %s\n", id);

    fmt->bf_id = id;
    fmt->bf_load = loadfunc;
    list_insert_head(&binfmt_list, &fmt->bf_link);

    return 0;
}

long binfmt_load(const char *filename, char *const *argv, char *const *envp,
                 uint64_t *rip, uint64_t *rsp)
{
    long fd = do_open(filename, O_RDONLY);
    if (fd < 0)
    {
        dbg(DBG_EXEC, "ERROR: exec failed to open file %s\n", filename);
        return fd;
    }
    file_t *file = fget((int)fd);
    long ret = 0;
    if (S_ISDIR(file->f_vnode->vn_mode))
    {
        ret = -EISDIR;
    }
    if (!ret && !S_ISREG(file->f_vnode->vn_mode))
    {
        ret = -EACCES;
    }
    fput(&file);
    if (ret)
    {
        do_close((int)fd);
        return ret;
    }

    list_iterate(&binfmt_list, fmt, binfmt_t, bf_link)
    {
        dbg(DBG_EXEC, "Trying to exec %s using binary loader %s\n", filename,
            fmt->bf_id);

        /* ENOEXE indicates that the given loader is unable to load
         * the given file, any other error indicates that the file
         * was recognized, but some other error existed which should
         * be returned to the user, only if all loaders specify ENOEXEC
         * do we actually return ENOEXEC */
        ret = fmt->bf_load(filename, (int)fd, argv, envp, rip, rsp);
        if (ret != -ENOEXEC)
        {
            do_close((int)fd);
        }
    }

    do_close((int)fd);
    return ret;
}
