//
// Tests some edge cases of s5fs
// Ported to user mode by twd in 7/2018
//

#ifdef __KERNEL__

#include "config.h"
#include "errno.h"
#include "globals.h"
#include "limits.h"

#include "test/usertest.h"

#include "util/debug.h"
#include "util/printf.h"
#include "util/string.h"

#include "fs/fcntl.h"
#include "fs/lseek.h"
#include "fs/s5fs/s5fs.h"
#include "fs/vfs_syscall.h"

#else

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <test/test.h>
#include <unistd.h>

#define do_write write
#define do_read read
#define do_lseek lseek
#define do_open(x, y) open((x), (y), 0777)
#define do_link link
#define do_unlink unlink
#define do_mknod mknod
#define do_close close
#define do_mkdir(x) mkdir((x), 0777)
#define do_chdir chdir
#define do_rmdir rmdir

#define S5_BLOCK_SIZE 4096
#define S5_MAX_FILE_BLOCKS 1052

#define KASSERT(x) test_assert(x, NULL)
#define dbg(code, fmt, args...) printf(fmt, ##args)

#endif

#define BUFSIZE 256
#define BIG_BUFSIZE 2056

#define S5_MAX_FILE_SIZE S5_BLOCK_SIZE *S5_MAX_FILE_BLOCKS

static void get_file_name(char *buf, size_t sz, int fileno)
{
    snprintf(buf, sz, "file%d", fileno);
}

// Write to a fail forever until it is either filled up or we get an error.
static int write_until_fail(int fd)
{
    size_t total_written = 0;
    char buf[BIG_BUFSIZE] = {42};
    while (total_written < S5_MAX_FILE_SIZE)
    {
        int res = do_write(fd, buf, BIG_BUFSIZE);
        if (res < 0)
        {
            return res;
        }
        total_written += res;
    }
    KASSERT(total_written == S5_MAX_FILE_SIZE);
    KASSERT(do_lseek(fd, 0, SEEK_END) == S5_MAX_FILE_SIZE);

    return 0;
}

// Read n bytes from the file, and check they're all 0
// We do this in increments of big_bufsize because we might want to read
// like a million bytes from the file
static int is_first_n_bytes_zero(int fd, int n)
{
    int total_read = 0;
    while (total_read < n)
    {
        int amt_to_read = MIN(BIG_BUFSIZE, n - total_read);
        char buf[BIG_BUFSIZE] = {1};
        int res = do_read(fd, buf, amt_to_read);
        if (res != amt_to_read)
        {
            dbg(DBG_TESTFAIL, "do_read result was %d\n", res);
            return 0;
            // KASSERT("Could not read from file" && 0);
        }
        total_read += res;

        // Check everything that we read is indeed 0
        for (int i = 0; i < amt_to_read; ++i)
        {
            if (buf[i] != 0)
            {
                dbg(DBG_TESTFAIL, "buf contains char %d\n", (int)buf[i]);
                return 0;
            }
        }
    }

    return 1;
}

static void test_running_out_of_inodes()
{
    // Open a ton of files until we get an error
    int res = -1;
    int fileno = 0;
    char filename[BUFSIZE];

    // open files til we get an error
    while (1)
    {
        get_file_name(filename, BUFSIZE, fileno);
        res = do_open(filename, O_RDONLY | O_CREAT);
        if (res >= 0)
        {
            fileno++;
            test_assert(do_close(res) == 0, "couldn't close");
        }
        else
        {
            break;
        }
    }
#ifdef __KERNEL__
    test_assert(res == -ENOSPC, "Did not get ENOSPC error");
#else
    test_assert(errno == ENOSPC, "Did not get ENOSPC error");
#endif

    // make sure mkdir fails now that we're out of inodes
    test_assert(do_mkdir("directory") < 0, "do_mkdir worked!?");
#ifdef __KERNEL__
    test_assert(res == -ENOSPC, "unexpected error");
#else
    test_assert(errno == ENOSPC, "unexpected error");
#endif

#ifdef __KERNEL__
    test_assert(do_mknod("nod", S_IFCHR, 123) != 0, "mknod worked!?");
    test_assert(res == -ENOSPC, "wrong error code");
#endif

    // the last file we tried to open failed
    fileno--;

    do
    {
        get_file_name(filename, BUFSIZE, fileno);
        res = do_unlink(filename);
        test_assert(res == 0, "couldnt unlink");
        fileno--;
    } while (fileno >= 0);

    // Now we've freed all the files, try to create another file
    int fd = do_open("file", O_RDONLY | O_CREAT);
    test_assert(fd >= 0, "Still cannot create files");
    test_assert(do_close(fd) == 0, "Could not do_close fd");
    test_assert(do_unlink("file") == 0, "Could not remove file");
}

static void test_filling_file()
{
    int res = 0;
    int fd = do_open("hugefile", O_RDWR | O_CREAT);
    KASSERT(fd >= 0);

    res = write_until_fail(fd);
    test_assert(res == 0, "Did not write to entire file");

    // make sure all other writes are unsuccessful/dont complete
    char buf[BIG_BUFSIZE] = {0};
    res = do_write(fd, buf, sizeof(buf));
    test_assert(res < 0, "Able to write although the file is full");
#ifdef __KERNEL__
    test_assert(res == -EFBIG || res == -EINVAL, "Wrong error code");
#else
    test_assert(errno == EFBIG || errno == EINVAL, "Wrong error code");
#endif

    test_assert(do_close(fd) == 0, "couldnt close hugefile");
    test_assert(do_unlink("hugefile") == 0, "couldnt unlink hugefile");
}

// Fill up the disk. Apparently to do this, we should need to fill up one
// entire file, then start to fill up another. We should eventually get
// the ENOSPC error
static void test_running_out_of_blocks()
{
    int res = 0;

    int fd1 = do_open("fullfile", O_RDWR | O_CREAT);

    res = write_until_fail(fd1);
    test_assert(res == 0, "Ran out of space quicker than we expected");

    int fd2 = do_open("partiallyfullfile", O_RDWR | O_CREAT);
    res = write_until_fail(fd2);
#ifdef __KERNEL__
    test_assert(res == -ENOSPC, "Did not get nospc error");
#else
    test_assert(errno == ENOSPC, "Did not get nospc error");
#endif

    test_assert(do_close(fd1) == 0, "could not close");
    test_assert(do_close(fd2) == 0, "could not close");

    test_assert(do_unlink("fullfile") == 0, "couldnt do_unlink file");
    test_assert(do_unlink("partiallyfullfile") == 0, "couldnt do_unlink file");
}

// Open a new file, write to some random address in the file,
// and make sure everything up to that is all 0s.
static int test_sparseness_direct_blocks()
{
    const char *filename = "sparsefile";
    int fd = do_open(filename, O_RDWR | O_CREAT);

    // Now write to some random address that'll be in a direct block
    const int addr = 10000;
    const char *b = "iboros";
    const int sz = strlen(b);

    test_assert(do_lseek(fd, addr, SEEK_SET) == addr, "couldnt seek");
    test_assert(do_write(fd, b, sz) == sz, "couldnt write to random address");

    test_assert(do_lseek(fd, 0, SEEK_SET) == 0, "couldnt seek back to begin");
    test_assert(is_first_n_bytes_zero(fd, addr) == 1, "sparseness don't work");

    // Get rid of this file
    test_assert(do_close(fd) == 0, "couldn't close file");
    test_assert(do_unlink(filename) == 0, "couldnt unlink file");

    return 0;
}

/*
 * Fixed by twd to do a better test in 7/2018
 */
static int test_sparseness_indirect_blocks()
{
    const char *filename = "bigsparsefile";
    int fd = do_open(filename, O_RDWR | O_CREAT);

    // first partially fill the first block of the file
    char randomgarbage[4050];
    test_assert(do_write(fd, randomgarbage, 4050) == 4050,
                "couldn't write to first block");

    // Now write to some random address that'll be in an indirect block
    const int addr = 1000000;
    const char *b = "iboros";
    const int sz = strlen(b);

    test_assert(do_lseek(fd, addr, SEEK_SET) == addr, "couldnt seek");
    test_assert(do_write(fd, b, sz) == sz, "couldnt write to random address");

    test_assert(do_lseek(fd, 4050, SEEK_SET) == 4050,
                "couldnt seek back to begin");
    test_assert(is_first_n_bytes_zero(fd, addr - 4050) == 1,
                "sparseness don't work");

    // Get rid of this file
    test_assert(do_close(fd) == 0, "couldn't close file");
    test_assert(do_unlink(filename) == 0, "couldnt unlink file");

    return 0;
}

#ifdef __KERNEL__
extern uint64_t jiffies;
#endif

static void seed_randomness()
{
#ifdef __KERNEL__
    srand(jiffies);
#else
    srand(time(NULL));
#endif
    rand();
}

#ifdef __KERNEL__
int s5fstest_main()
#else

int main()
#endif
{
    dbg(DBG_TEST, "Starting S5FS test\n");

    test_init();
    seed_randomness();

    KASSERT(do_mkdir("s5fstest") == 0);
    KASSERT(do_chdir("s5fstest") == 0);
    dbg(DBG_TEST, "Test dir initialized\n");

    dbg(DBG_TEST, "Testing sparseness for direct blocks\n");
    test_sparseness_direct_blocks();
    dbg(DBG_TEST, "Testing sparseness for indirect blocks\n");
    test_sparseness_indirect_blocks();

    dbg(DBG_TEST, "Testing running out of inodes\n");
    test_running_out_of_inodes();
    dbg(DBG_TEST, "Testing filling a file to max capacity\n");
    test_filling_file();
    dbg(DBG_TEST, "Testing using all available blocks on disk\n");
    test_running_out_of_blocks();

    test_assert(do_chdir("..") == 0, "");
    test_assert(do_rmdir("s5fstest") == 0, "");

    test_fini();

    return 0;
}
