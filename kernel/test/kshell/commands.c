#include "commands.h"
#include "errno.h"

#include "command.h"

#ifdef __VFS__

#include "fs/fcntl.h"
#include "fs/vfs_syscall.h"
#include "fs/vnode.h"

#endif

#include "test/kshell/io.h"

#include "util/debug.h"
#include "util/string.h"

list_t kshell_commands_list = LIST_INITIALIZER(kshell_commands_list);

long kshell_help(kshell_t *ksh, size_t argc, char **argv)
{
    /* Print a list of available commands */
    char spaces[KSH_CMD_NAME_LEN];
    memset(spaces, ' ', KSH_CMD_NAME_LEN);

    kprintf(ksh, "Available commands:\n");
    list_iterate(&kshell_commands_list, cmd, kshell_command_t,
                 kc_commands_link)
    {
        KASSERT(NULL != cmd);
        size_t namelen = strnlen(cmd->kc_name, KSH_CMD_NAME_LEN);
        spaces[KSH_CMD_NAME_LEN - namelen] = '\0';
        kprintf(ksh, "%s%s%s\n", cmd->kc_name, spaces, cmd->kc_desc);
        spaces[KSH_CMD_NAME_LEN - namelen] = ' ';
    }

    return 0;
}

long kshell_exit(kshell_t *ksh, size_t argc, char **argv)
{
    panic("kshell: kshell_exit should NEVER be called");
}

long kshell_clear(kshell_t *ksh, size_t argc, char **argv)
{
    kprintf(ksh, "\033[2J\033[1;1H");

    // kprintf(ksh, "\033[10A");
    return 0;
}

long kshell_halt(kshell_t *ksh, size_t argc, char **argv)
{
    proc_kill_all();
    return 0;
}

long kshell_echo(kshell_t *ksh, size_t argc, char **argv)
{
    if (argc == 1)
    {
        kprintf(ksh, "\n");
    }
    else
    {
        for (size_t i = 1; i < argc - 1; i++)
        {
            kprintf(ksh, "%s ", argv[i]);
        }
        kprintf(ksh, "%s\n", argv[argc - 1]);
    }

    return 0;
}

#ifdef __VFS__

long kshell_cat(kshell_t *ksh, size_t argc, char **argv)
{
    if (argc < 2)
    {
        kprintf(ksh, "Usage: cat <files>\n");
        return 0;
    }

    char buf[KSH_BUF_SIZE];
    for (size_t i = 1; i < argc; i++)
    {
        int fd = (int)do_open(argv[i], O_RDONLY);
        if (fd < 0)
        {
            kprintf(ksh, "Error opening file: %s\n", argv[i]);
            continue;
        }

        long retval;
        while ((retval = do_read(fd, buf, KSH_BUF_SIZE)) > 0)
        {
            retval = kshell_write_all(ksh, buf, (size_t)retval);
            if (retval < 0)
                break;
        }
        if (retval < 0)
        {
            kprintf(ksh, "Error reading or writing %s: %s\n", argv[i], strerror((int)-retval));
        }

        retval = do_close(fd);
        if (retval < 0)
        {
            panic("kshell: Error closing file %s: %s\n", argv[i],
                  strerror((int)-retval));
        }
    }

    return 0;
}

long kshell_ls(kshell_t *ksh, size_t argc, char **argv)
{
    size_t arglen;
    long ret;
    int fd;
    dirent_t dirent;
    stat_t statbuf;
    char direntname[KSH_BUF_SIZE];

    memset(direntname, '\0', KSH_BUF_SIZE);

    if (argc > 2)
    {
        kprintf(ksh, "Usage: ls <directory>\n");
        return 0;
    }
    else if (argc == 2)
    {
        if ((ret = do_stat(argv[1], &statbuf)) < 0)
        {
            if (ret == -ENOENT)
            {
                kprintf(ksh, "%s does not exist\n", argv[1]);
                return 0;
            }
            else
            {
                return ret;
            }
        }
        if (!S_ISDIR(statbuf.st_mode))
        {
            kprintf(ksh, "%s is not a directory\n", argv[1]);
            return 0;
        }

        fd = (int)do_open(argv[1], O_RDONLY);
        if (fd < 0)
        {
            kprintf(ksh, "Could not find directory: %s\n", argv[1]);
            return 0;
        }
        arglen = strnlen(argv[1], KSH_BUF_SIZE);
    }
    else
    {
        KASSERT(argc == 1);
        fd = (int)do_open(".", O_RDONLY);
        if (fd < 0)
        {
            kprintf(ksh, "Could not find directory: .\n");
            return 0;
        }
        arglen = 1;
    }

    if (argc == 2)
        memcpy(direntname, argv[1], arglen);
    else
        direntname[0] = '.';

    direntname[arglen] = '/';
    direntname[arglen + NAME_LEN + 1] = '\0';

    while ((ret = do_getdent(fd, &dirent)) == sizeof(dirent_t))
    {
        memcpy(direntname + arglen + 1, dirent.d_name, NAME_LEN + 1);
        ret = do_stat(direntname, &statbuf);
        if (ret < 0)
        {
            kprintf(ksh, "Error stat\'ing `%s`: %s\n", dirent.d_name, strerror((int)-ret));
            continue;
        }
        if (S_ISDIR(statbuf.st_mode))
        {
            kprintf(ksh, "%s/\n", dirent.d_name);
        }
        else
        {
            kprintf(ksh, "%s\n", dirent.d_name);
        }
    }

    do_close(fd);
    return ret;
}

long kshell_cd(kshell_t *ksh, size_t argc, char **argv)
{
    KASSERT(ksh && argc && argv);
    if (argc < 2)
    {
        kprintf(ksh, "Usage: cd <directory>\n");
        return 0;
    }

    long ret = do_chdir(argv[1]);
    if (ret < 0)
    {
        kprintf(ksh, "cd: `%s`: %s\n", argv[1], strerror((int)-ret));
    }
    return 0;
}

long kshell_rm(kshell_t *ksh, size_t argc, char **argv)
{
    KASSERT(ksh && argc && argv);

    if (argc < 2)
    {
        kprintf(ksh, "Usage: rm <file>\n");
        return 0;
    }

    long ret = do_unlink(argv[1]);
    if (ret < 0)
    {
        kprintf(ksh, "rm: `%s`: %s\n", argv[1], strerror((int)-ret));
    }

    return 0;
}

long kshell_link(kshell_t *ksh, size_t argc, char **argv)
{
    KASSERT(ksh && argc && argv);

    if (argc < 3)
    {
        kprintf(ksh, "Usage: link <src> <dst>\n");
        return 0;
    }

    long ret = do_link(argv[1], argv[2]);
    if (ret < 0)
    {
        kprintf(ksh, "Error linking %s to %s: %s\n", argv[1], argv[2], strerror((int)-ret));
    }

    return 0;
}

long kshell_rmdir(kshell_t *ksh, size_t argc, char **argv)
{
    KASSERT(ksh && argc && argv);
    if (argc < 2)
    {
        kprintf(ksh, "Usage: rmdir DIRECTORY...\n");
        return 1;
    }

    long exit_val = 0;
    for (size_t i = 1; i < argc; i++)
    {
        long ret = do_rmdir(argv[i]);
        if (ret < 0)
        {
            kprintf(ksh, "rmdir: failed to remove directory `%s': %s\n",
                    argv[i], strerror((int)-ret));
            exit_val = 1;
        }
    }

    return exit_val;
}

long kshell_mkdir(kshell_t *ksh, size_t argc, char **argv)
{
    KASSERT(ksh && argc && argv);
    if (argc < 2)
    {
        kprintf(ksh, "Usage: mkdir DIRECTORY...\n");
        return 1;
    }

    long exit_val = 0;
    for (size_t i = 1; i < argc; i++)
    {
        long ret = do_mkdir(argv[i]);
        if (ret < 0)
        {
            kprintf(ksh, "mkdir: failed to create directory `%s': %s\n",
                    argv[i], strerror((int)-ret));
            exit_val = 1;
        }
    }

    return exit_val;
}

static const char *get_file_type_str(int mode)
{
    if (S_ISCHR(mode))
    {
        return "character special file";
    }
    else if (S_ISDIR(mode))
    {
        return "directory";
    }
    else if (S_ISBLK(mode))
    {
        return "block special file";
    }
    else if (S_ISREG(mode))
    {
        return "regular file";
    }
    else if (S_ISLNK(mode))
    {
        return "symbolic link";
    }
    else
    {
        return "unknown";
    }
}

long kshell_stat(kshell_t *ksh, size_t argc, char **argv)
{
    KASSERT(ksh && argc && argv);
    long exit_val = 0;

    if (argc < 2)
    {
        kprintf(ksh, "Usage: stat FILE...\n");
        return 1;
    }

    for (size_t i = 1; i < argc; i++)
    {
        stat_t buf;
        long ret = do_stat(argv[i], &buf);
        if (ret < 0)
        {
            kprintf(ksh, "Cannot stat `%s': %s\n", argv[i],
                    strerror((int)-ret));
            exit_val = 1;
            continue;
        }
        const char *file_type_str = get_file_type_str(buf.st_mode);
        kprintf(ksh, "File: `%s'\n", argv[i]);
        kprintf(ksh, "Size: %d\n", buf.st_size);
        kprintf(ksh, "Blocks: %d\n", buf.st_blocks);
        kprintf(ksh, "IO Block: %d\n", buf.st_blksize);
        kprintf(ksh, "%s\n", file_type_str);
        kprintf(ksh, "Inode: %d\n", buf.st_ino);
        kprintf(ksh, "Links: %d\n", buf.st_nlink);
    }

    return exit_val;
}

long vfstest_main(int, void *);

long kshell_vfs_test(kshell_t *ksh, size_t argc, char **argv)
{
    kprintf(ksh, "TEST VFS: Testing... Please wait.\n");

    long ret = vfstest_main(1, NULL);

    kprintf(ksh, "TEST VFS: testing complete, check console for results\n");

    return ret;
}

#endif

#ifdef __S5FS__

long s5fstest_main(int, void *);

long kshell_s5fstest(kshell_t *ksh, size_t argc, char **argv)
{
    kprintf(ksh, "TEST S5FS: Testing... Please wait.\n");

    long ret = s5fstest_main(1, NULL);

    kprintf(ksh, "TEST S5FS: testing complete, check console for results\n");

    return ret;
}

#endif
