/*
 * The elf32 loader (the basis for this file) was modified by twd in 7/2018 so
 * that it lays out the address space in a more Unix-like fashion (e.g., the
 * stack is at the top of user memory, text is near the bottom).
 *
 * This loader (and the elf32 loader) are not strictly ABI compliant.  See the
 * Intel i386 ELF supplement pp 54-59 and AMD64 ABI Draft 0.99.6 page 29 for
 * what initial process stacks are supposed to look like after the iret(q) in
 * userland_entry is executed.  The following would be required (but not
 * necessarily sufficient!) for full compliance:
 *
 * 1) Remove the pointers to argv, envp, and auxv from the initial stack.
 * 2) Have __libc_static_entry (static entry) and _ldloadrtld (callee of dynamic
 * entry) calculate those pointers and place them on the stack (x86) or in
 * registers (x86-64) along with argc as arguments to main. 3) Ensure that the
 * stack pointer is 4 byte (x86) or 16 byte (x86-64) aligned by padding the end
 * of the arguments being written to the stack with zeros. 4) Have the stack
 * pointer point to argc, rather than a garbage return address. 5) Have
 * __libc_static_entry and _bootstrap (ld-weenix) respect this change.
 */

#include "errno.h"
#include "globals.h"

#include "main/inits.h"

#include "mm/kmalloc.h"
#include "mm/mm.h"
#include "mm/mman.h"
#include "mm/tlb.h"

#include "api/binfmt.h"
#include "api/elf.h"

#include "util/debug.h"
#include "util/string.h"

#include "fs/fcntl.h"
#include "fs/file.h"
#include "fs/lseek.h"
#include "fs/vfs_syscall.h"

static long _elf64_platform_check(const Elf64_Ehdr *header)
{
    return (EM_X86_64 == header->e_machine)              // machine
           && (ELFCLASS64 == header->e_ident[EI_CLASS])  // 32 or 64 bit
           && (ELFDATA2LSB == header->e_ident[EI_DATA]); // endianness
}

/* Helper function for the ELF loader. Maps the specified segment
 * of the program header from the given file in to the given address
 * space with the given memory offset (in pages). On success returns 0,
 * otherwise returns a negative error code for the ELF loader to return. Note
 * that since any error returned by this function should cause the ELF loader to
 * give up, it is acceptable for the address space to be modified after
 * returning an error. Note that memoff can be negative */
static long _elf64_map_segment(vmmap_t *map, vnode_t *file, int64_t memoff,
                               const Elf64_Phdr *segment)
{
    /* calculate starting virtual address of segment e*/
    uintptr_t addr;
    if (memoff < 0)
    {
        KASSERT(ADDR_TO_PN(segment->p_vaddr) > (uint64_t)-memoff);
        addr = (uintptr_t)segment->p_vaddr - (uintptr_t)PN_TO_ADDR(-memoff);
    }
    else
    {
        addr = (uintptr_t)segment->p_vaddr + (uintptr_t)PN_TO_ADDR(memoff);
    }
    uint64_t off = segment->p_offset;
    uint64_t memsz = segment->p_memsz;
    uint64_t filesz = segment->p_filesz;

    dbg(DBG_ELF,
        "Mapping program segment: type %#x, offset %#16lx,"
        " vaddr %#16lx, filesz %#lx, memsz %#lx, flags %#x, align %#lx\n",
        segment->p_type, segment->p_offset, segment->p_vaddr, segment->p_filesz,
        segment->p_memsz, segment->p_flags, segment->p_align);

    /* check for bad data in the segment header */
    if ((segment->p_align % PAGE_SIZE))
    {
        dbg(DBG_ELF, "ERROR: segment not aligned on page\n");
        return -ENOEXEC;
    }
    else if (filesz > memsz)
    {
        dbg(DBG_ELF, "ERROR: segment file size is greater than memory size\n");
        return -ENOEXEC;
    }
    else if (PAGE_OFFSET(addr) != PAGE_OFFSET(off))
    {
        dbg(DBG_ELF,
            "ERROR: segment address and offset are not aligned correctly\n");
        return -ENOEXEC;
    }

    /* calculate segment permissions */
    int perms = 0;
    if (PF_R & segment->p_flags)
    {
        perms |= PROT_READ;
    }
    if (PF_W & segment->p_flags)
    {
        perms |= PROT_WRITE;
    }
    if (PF_X & segment->p_flags)
    {
        perms |= PROT_EXEC;
    }

    if (filesz > 0)
    {
        /* something needs to be mapped from the file */
        /* start from the starting address and include enough pages to
         * map all filesz bytes of the file */
        uint64_t lopage = ADDR_TO_PN(addr);
        uint64_t npages = ADDR_TO_PN(addr + filesz - 1) - lopage + 1;
        off_t fileoff = (off_t)PAGE_ALIGN_DOWN(off);

        if (!vmmap_is_range_empty(map, lopage, npages))
        {
            dbg(DBG_ELF, "ERROR: ELF file contains overlapping segments\n");
            return -ENOEXEC;
        }
        long ret = vmmap_map(map, file, lopage, npages, perms,
                             MAP_PRIVATE | MAP_FIXED, fileoff, 0, NULL);
        if (ret)
            return ret;
        dbg(DBG_ELF,
            "Mapped segment of length %lu pages at %#lx, memoff = %#lx\n",
            npages, addr, memoff);
    }

    if (memsz > filesz)
    {
        /* there is left over memory in the segment which must
         * be initialized to 0 (anonymously mapped) */
        uint64_t lopage = ADDR_TO_PN(
            addr +
            filesz); // the first page containing data not stored in the file
        uint64_t npages =
            ADDR_TO_PN(PAGE_ALIGN_UP(addr + memsz)) -
            lopage; // the first page totally unused by memory, minus low page

        /* check for overlapping mappings, considering the case where lopage
         * contains file data and the case where it doesn't*/
        if (PAGE_ALIGNED(addr + filesz) &&
            !vmmap_is_range_empty(map, lopage, npages))
        {
            dbg(DBG_ELF, "ERROR: ELF file contains overlapping segments\n");
            return -ENOEXEC;
        }
        if (!PAGE_ALIGNED(addr + filesz) && npages > 1 &&
            !vmmap_is_range_empty(map, lopage + 1, npages - 1))
        {
            dbg(DBG_ELF, "ERROR: ELF file contains overlapping segments\n");
            return -ENOEXEC;
        }
        long ret = vmmap_map(map, NULL, lopage, npages, perms,
                             MAP_PRIVATE | MAP_FIXED, 0, 0, NULL);
        if (ret)
            return ret;
        if (!PAGE_ALIGNED(addr + filesz) && filesz > 0)
        {
            /* In this case, we have accidentally zeroed too much of memory, as
             * we zeroed all memory in the page containing addr + filesz.
             * However, the remaining part of the data is not a full page, so we
             * should not just map in another page (as there could be garbage
             * after addr+filesz). For instance, consider the data-bss boundary
             * (c.f. Intel x86 ELF supplement pp. 82).
             * To fix this, we need to read in the contents of the file manually
             * and put them at that user space addr in the anon map we just
             * added. */
            void *buf = page_alloc();
            if (!buf)
                return -ENOMEM;

            vlock(file);
            ret = file->vn_ops->read(file,
                                     (size_t)PAGE_ALIGN_DOWN(off + filesz - 1),
                                     buf, PAGE_OFFSET(addr + filesz));
            if (ret >= 0)
            {
                KASSERT((uintptr_t)ret == PAGE_OFFSET(addr + filesz));
                ret = vmmap_write(map, PAGE_ALIGN_DOWN(addr + filesz - 1), buf,
                                  PAGE_OFFSET(addr + filesz));
            }
            vunlock(file);
            page_free(buf);
            return ret;
        }
    }
    return 0;
}

/* Read in the given fd's ELF header into the location pointed to by the given
 * argument and does some basic checks that it is a valid ELF file, is an
 * executable, and is for the correct platform
 * interp is 1 if we are loading an interpreter, 0 otherwise
 * Returns 0 on success, -errno on failure. Returns the ELF header in the header
 * argument. */
static long _elf64_load_ehdr(int fd, Elf64_Ehdr *header, int interp)
{
    long ret;
    memset(header, 0, sizeof(*header));

    /* Preliminary check that this is an ELF file */
    ret = do_read(fd, header, sizeof(*header));
    if (ret < 0)
        return ret;
    if ((ret < SELFMAG) || memcmp(&header->e_ident[0], ELFMAG, SELFMAG) != 0)
    {
        dbg(DBG_ELF, "ELF load failed: no magic number present\n");
        return -ENOEXEC;
    }
    if (ret < header->e_ehsize)
    {
        dbg(DBG_ELF, "ELF load failed: bad file size\n");
        return -ENOEXEC;
    }
    /* Log information about the file */
    dbg(DBG_ELF, "loading ELF file\n");
    dbgq(DBG_ELF, "ELF Header Information:\n");
    dbgq(DBG_ELF, "Version: %d\n", (int)header->e_ident[EI_VERSION]);
    dbgq(DBG_ELF, "Class:   %d\n", (int)header->e_ident[EI_CLASS]);
    dbgq(DBG_ELF, "Data:    %d\n", (int)header->e_ident[EI_DATA]);
    dbgq(DBG_ELF, "Type:    %d\n", (int)header->e_type);
    dbgq(DBG_ELF, "Machine: %d\n", (int)header->e_machine);

    /* Check that the ELF file is executable and targets
     * the correct platform */
    if (interp && header->e_type != ET_DYN)
    {
        dbg(DBG_ELF,
            "ELF load failed: interpreter is not a shared object file\n");
        return -ENOEXEC;
    }
    if (!interp && header->e_type != ET_EXEC)
    {
        dbg(DBG_ELF, "ELF load failed: not executable ELF\n");
        return -ENOEXEC;
    }
    if (!_elf64_platform_check(header))
    {
        dbg(DBG_ELF, "ELF load failed: incorrect platform\n");
        return -ENOEXEC;
    }
    return 0;
}

/* Loads the program header tables from from the ELF file specified by
 * the open file descriptor fd. header should point to the header information
 * for that ELF file. pht is a buffer of size size. It must be large enough
 * to hold the program header tables (whose size can be determined from
 * the ELF header).
 *
 * Returns 0 on success or -errno on error. */
static long _elf64_load_phtable(int fd, Elf64_Ehdr *header, char *pht,
                                size_t size)
{
    size_t phtsize = header->e_phentsize * header->e_phnum;
    KASSERT(phtsize <= size);
    /* header->e_phoff is a uint64_t cast to int.  since the max file size on
     * s5fs is way smaller than uint32_t, offsets in practice should never
     * cause this cast to behave badly, although if weenix ever adds support
     * for very large (> 4GB) files, this will be a bug.
     */
    long ret = do_lseek(fd, (int)(header->e_phoff), SEEK_SET);
    if (ret < 0)
        return ret;

    ret = do_read(fd, pht, phtsize);
    if (ret < 0)
        return ret;

    KASSERT((size_t)ret <= phtsize);
    if ((size_t)ret < phtsize)
    {
        return -ENOEXEC;
    }
    return 0;
}

/* Maps the PT_LOAD segments for an ELF file into the given address space.
 * vnode should be the open vnode of the ELF file.
 * map is the address space to map the ELF file into.
 * header is the ELF file's header.
 * pht is the full program header table.
 * memoff is the difference (in pages) between the desired base address and the
 * base address given in the ELF file (usually 0x8048094)
 *
 * Returns the number of segments loaded on success, -errno on failure. */
static long _elf64_map_progsegs(vnode_t *vnode, vmmap_t *map,
                                Elf64_Ehdr *header, char *pht, int64_t memoff)
{
    long ret = 0;

    long loadcount = 0;
    for (uint32_t i = 0; i < header->e_phnum; i++)
    {
        Elf64_Phdr *phtentry = (Elf64_Phdr *)(pht + i * header->e_phentsize);
        if (phtentry->p_type == PT_LOAD)
        {
            ret = _elf64_map_segment(map, vnode, memoff, phtentry);
            if (ret)
                return ret;
            loadcount++;
        }
    }

    if (!loadcount)
    {
        dbg(DBG_ELF, "ERROR: ELF file contained no loadable sections\n");
        return -ENOEXEC;
    }
    return loadcount;
}

/* Locates the program header for the interpreter in the given list of program
 * headers through the phinterp out-argument. Returns 0 on success (even if
 * there is no interpreter) or -errno on error. If there is no interpreter
 * section then phinterp is set to NULL. If there is more than one interpreter
 * then -EINVAL is returned. */
static long _elf64_find_phinterp(Elf64_Ehdr *header, char *pht,
                                 Elf64_Phdr **phinterp)
{
    *phinterp = NULL;

    for (uint32_t i = 0; i < header->e_phnum; i++)
    {
        Elf64_Phdr *phtentry = (Elf64_Phdr *)(pht + i * header->e_phentsize);
        if (phtentry->p_type == PT_INTERP)
        {
            if (!*phinterp)
            {
                *phinterp = phtentry;
            }
            else
            {
                dbg(DBG_ELF, "ELF load failed: multiple interpreters\n");
                return -EINVAL;
            }
        }
    }
    return 0;
}

/* Calculates the lower and upper virtual addresses that the given program
 * header table would load into if _elf64_map_progsegs were called. We traverse
 * all the program segments of type PT_LOAD and look at p_vaddr and p_memsz
 * Return the low and high vaddrs in the given arguments if they are non-NULL.
 * The high vaddr is one plus the highest vaddr used by the program. */
static void _elf64_calc_progbounds(Elf64_Ehdr *header, char *pht, void **low,
                                   void **high)
{
    Elf64_Addr curlow = (Elf64_Addr)-1;
    Elf64_Addr curhigh = 0;
    for (uint32_t i = 0; i < header->e_phnum; i++)
    {
        Elf64_Phdr *phtentry = (Elf64_Phdr *)(pht + i * header->e_phentsize);
        if (phtentry->p_type == PT_LOAD)
        {
            if (phtentry->p_vaddr < curlow)
            {
                curlow = phtentry->p_vaddr;
            }
            if (phtentry->p_vaddr + phtentry->p_memsz > curhigh)
            {
                curhigh = phtentry->p_vaddr + phtentry->p_memsz;
            }
        }
    }
    if (low)
    {
        *low = (void *)curlow;
    }
    if (high)
    {
        *high = (void *)curhigh;
    }
}

/* Calculates the total size of all the arguments that need to be placed on the
 * user stack before execution can begin. See AMD64 ABI Draft 0.99.6 page 29
 * Returns total size on success. Returns the number of non-NULL entries in
 * argv, envp, and auxv in argc, envc, and auxc arguments, respectively */
static size_t _elf64_calc_argsize(char *const argv[], char *const envp[],
                                  Elf64_auxv_t *auxv, size_t phtsize,
                                  size_t *argc, size_t *envc, size_t *auxc)
{
    size_t size = 0;
    size_t i;
    /* All strings in argv */
    for (i = 0; argv[i]; i++)
    {
        size += strlen(argv[i]) + 1; /* null terminator */
    }
    if (argc)
    {
        *argc = i;
    }
    /* argv itself (+ null terminator) */
    size += (i + 1) * sizeof(char *);

    /* All strings in envp */
    for (i = 0; envp[i] != NULL; i++)
    {
        size += strlen(envp[i]) + 1; /* null terminator */
    }
    if (envc != NULL)
    {
        *envc = i;
    }
    /* envp itself (+ null terminator) */
    size += (i + 1) * sizeof(char *);

    /* The only extra-space-consuming entry in auxv is AT_PHDR, as if we find
     * that entry we'll need to put the program header table on the stack */
    for (i = 0; auxv[i].a_type != AT_NULL; i++)
    {
        if (auxv[i].a_type == AT_PHDR)
        {
            size += phtsize;
        }
    }
    if (auxc)
    {
        *auxc = i;
    }
    /* auxv itself (+ null terminator) */
    size += (i + 1) * sizeof(Elf64_auxv_t);

    /* argc - reserving 8 bytes for alignment purposes */
    size += sizeof(int64_t);
    /* argv, envp, and auxv pointers (as passed to main) */
    size += 3 * sizeof(void *);

    /*
     * cjm5: the above isn't strictly ABI compliant.  normally the userspace
     * wrappers to main() (__libc_static_entry or _bootstrap for ld-weenix) are
     * responsible for calculating *argv, *envp, *and *auxv to pass to main().
     * It's easier to do it here, though.
     */

    return size;
}

/* Copies the arguments that must be on the stack prior to execution onto the
 * user stack. This should never fail.
 * arglow:   low address on the user stack where we should start the copying
 * argsize:  total size of everything to go on the stack
 * buf:      a kernel buffer at least as big as argsize (for convenience)
 * argv, envp, auxv: various vectors of stuff (to go on the stack)
 * argc, envc, auxc: number of non-NULL entries in argv, envp, auxv,
 *                   respectively (to avoid recomputing them)
 * phtsize: the size of the program header table (to avoid recomputing)
 * c.f. Intel i386 ELF supplement pp 54-59 and AMD64 ABI Draft 0.99.6 page 29
 */
static void _elf64_load_args(vmmap_t *map, void *arglow, size_t argsize,
                             char *buf, char *const argv[], char *const envp[],
                             Elf64_auxv_t *auxv, size_t argc, size_t envc,
                             size_t auxc, size_t phtsize)
{
    dbg(DBG_ELF,
        "Loading initial stack contents at 0x%p, argc = %lu, envc = %lu, auxc "
        "= %lu\n",
        arglow, argc, envc, auxc);

    size_t i;

    /* Copy argc: in x86-64, this is an eight-byte value, despite being treated
     * as an int in a C main() function. See AMD64 ABI Draft 0.99.6 page 29 */
    *((int64_t *)buf) = (int64_t)argc;

    /* Calculate where the strings / tables pointed to by the vectors start */
    size_t veclen = (argc + 1 + envc + 1) * sizeof(char *) +
                    (auxc + 1) * sizeof(Elf64_auxv_t);

    char *vecstart =
        buf + sizeof(int64_t) +
        3 * sizeof(void *); /* Beginning of argv (in kernel buffer) */

    char *vvecstart =
        ((char *)arglow) + sizeof(int64_t) +
        3 * sizeof(void *); /* Beginning of argv (in user space) */

    char *strstart = vecstart + veclen; /* Beginning of first string pointed to
                                           by argv (in kernel buffer) */

    /* Beginning of first string pointed to by argv (in user space) */
    char *vstrstart = vvecstart + veclen;

    /*
     * cjm5: since the first 6 arguments that can fit in registers are placed
     * there in x86-64, __libc_static_entry (and ld-weenix, if it is ever ported
     * to x86-64) have to take the following pointers off the stack and move
     * them and argc into the first 4 argument registers before calling main().
     */

    /* Copy over pointer to argv */
    *(char **)(buf + 8) = vvecstart;
    /* Copy over pointer to envp */
    *(char **)(buf + 16) = vvecstart + (argc + 1) * sizeof(char *);
    /* Copy over pointer to auxv */
    *(char **)(buf + 24) = vvecstart + (argc + 1 + envc + 1) * sizeof(char *);

    /* Copy over argv along with every string in it */
    for (i = 0; i < argc; i++)
    {
        size_t len = strlen(argv[i]) + 1;
        strcpy(strstart, argv[i]);
        /* Remember that we need to use the virtual address of the string */
        *(char **)vecstart = vstrstart;
        strstart += len;
        vstrstart += len;
        vecstart += sizeof(char *);
    }
    /* null terminator of argv */
    *(char **)vecstart = NULL;
    vecstart += sizeof(char *);

    /* Copy over envp along with every string in it */
    for (i = 0; i < envc; i++)
    {
        size_t len = strlen(envp[i]) + 1;
        strcpy(strstart, envp[i]);
        /* Remember that we need to use the virtual address of the string */
        *(char **)vecstart = vstrstart;
        strstart += len;
        vstrstart += len;
        vecstart += sizeof(char *);
    }
    /* null terminator of envp */
    *(char **)vecstart = NULL;
    vecstart += sizeof(char *);

    /* Copy over auxv along with the program header (if we find it) */
    for (i = 0; i < auxc; i++)
    {
        /* Copy over the auxv entry */
        memcpy(vecstart, &auxv[i], sizeof(Elf64_auxv_t));
        /* Check if it points to the program header */
        if (auxv[i].a_type == AT_PHDR)
        {
            /* Copy over the program header table */
            memcpy(strstart, auxv[i].a_un.a_ptr, (size_t)phtsize);
            /* And modify the address */
            ((Elf64_auxv_t *)vecstart)->a_un.a_ptr = vstrstart;
        }
        vecstart += sizeof(Elf64_auxv_t);
    }
    /* null terminator of auxv */
    ((Elf64_auxv_t *)vecstart)->a_type = NULL;

    /* Finally, we're done copying into the kernel buffer. Now just copy the
     * kernel buffer into user space */
    long ret = vmmap_write(map, arglow, buf, argsize);
    /* If this failed, we must have set up the address space wrong... */
    KASSERT(!ret);
}

static long _elf64_load(const char *filename, int fd, char *const argv[],
                        char *const envp[], uint64_t *rip, uint64_t *rsp)
{
    long ret = 0;
    Elf64_Ehdr header;
    Elf64_Ehdr interpheader;

    /* variables to clean up on failure */
    vmmap_t *map = NULL;
    file_t *file = NULL;
    char *pht = NULL;
    char *interpname = NULL;
    long interpfd = -1;
    file_t *interpfile = NULL;
    char *interppht = NULL;
    Elf64_auxv_t *auxv = NULL;
    char *argbuf = NULL;

    uintptr_t entry;

    file = fget(fd);
    if (!file)
        return -EBADF;

    /* Load and verify the ELF header */
    ret = _elf64_load_ehdr(fd, &header, 0);
    if (ret)
        goto done;

    map = vmmap_create();
    if (!map)
    {
        ret = -ENOMEM;
        goto done;
    }

    // Program header table entry size multiplied by
    // number of entries.
    size_t phtsize = header.e_phentsize * header.e_phnum;
    pht = kmalloc(phtsize);
    if (!pht)
    {
        ret = -ENOMEM;
        goto done;
    }
    /* Read in the program header table */
    ret = _elf64_load_phtable(fd, &header, pht, phtsize);
    if (ret)
        goto done;

    /* Load the segments in the program header table */
    ret = _elf64_map_progsegs(file->f_vnode, map, &header, pht, 0);
    if (ret < 0)
        goto done;

    /* Check if program requires an interpreter */
    Elf64_Phdr *phinterp = NULL;
    ret = _elf64_find_phinterp(&header, pht, &phinterp);
    if (ret)
        goto done;

    /* Calculate program bounds for future reference */
    void *proglow;
    void *proghigh;
    _elf64_calc_progbounds(&header, pht, &proglow, &proghigh);

    entry = (uintptr_t)header.e_entry;

    /* if an interpreter was requested load it */
    if (phinterp)
    {
        /* read the file name of the interpreter from the binary */
        ret = do_lseek(fd, (int)(phinterp->p_offset), SEEK_SET);
        if (ret < 0)
            goto done;

        interpname = kmalloc(phinterp->p_filesz);
        if (!interpname)
        {
            ret = -ENOMEM;
            goto done;
        }
        ret = do_read(fd, interpname, phinterp->p_filesz);
        if (ret < 0)
            goto done;

        if ((size_t)ret != phinterp->p_filesz)
        {
            ret = -ENOEXEC;
            goto done;
        }

        /* open the interpreter */
        dbgq(DBG_ELF, "ELF Interpreter: %*s\n", (int)phinterp->p_filesz,
             interpname);
        interpfd = do_open(interpname, O_RDONLY);
        if (interpfd < 0)
        {
            ret = interpfd;
            goto done;
        }
        kfree(interpname);
        interpname = NULL;

        interpfile = fget((int)interpfd);
        KASSERT(interpfile);

        /* Load and verify the interpreter ELF header */
        ret = _elf64_load_ehdr((int)interpfd, &interpheader, 1);
        if (ret)
            goto done;

        size_t interpphtsize = interpheader.e_phentsize * interpheader.e_phnum;
        interppht = kmalloc(interpphtsize);
        if (!interppht)
        {
            ret = -ENOMEM;
            goto done;
        }
        /* Read in the program header table */
        ret = _elf64_load_phtable((int)interpfd, &interpheader, interppht,
                                  interpphtsize);
        if (ret)
            goto done;

        /* Interpreter shouldn't itself need an interpreter */
        Elf64_Phdr *interpphinterp;
        ret = _elf64_find_phinterp(&interpheader, interppht, &interpphinterp);
        if (ret)
            goto done;

        if (interpphinterp)
        {
            ret = -EINVAL;
            goto done;
        }

        /* Calculate the interpreter program size */
        void *interplow;
        void *interphigh;
        _elf64_calc_progbounds(&interpheader, interppht, &interplow,
                               &interphigh);
        uint64_t interpnpages =
            ADDR_TO_PN(PAGE_ALIGN_UP(interphigh)) - ADDR_TO_PN(interplow);

        /* Find space for the interpreter */
        /* This is the first pn at which the interpreter will be mapped */
        uint64_t interppagebase =
            (uint64_t)vmmap_find_range(map, interpnpages, VMMAP_DIR_HILO);
        if (interppagebase == ~0UL)
        {
            ret = -ENOMEM;
            goto done;
        }

        /* Base address at which the interpreter begins on that page */
        void *interpbase = (void *)((uintptr_t)PN_TO_ADDR(interppagebase) +
                                    PAGE_OFFSET(interplow));

        /* Offset from "expected base" in number of pages */
        int64_t interpoff =
            (int64_t)interppagebase - (int64_t)ADDR_TO_PN(interplow);

        entry = (uintptr_t)interpbase +
                ((uintptr_t)interpheader.e_entry - (uintptr_t)interplow);

        /* Load the interpreter program header and map in its segments */
        ret = _elf64_map_progsegs(interpfile->f_vnode, map, &interpheader,
                                  interppht, interpoff);
        if (ret < 0)
            goto done;

        /* Build the ELF aux table */
        /* Need to hold AT_PHDR, AT_PHENT, AT_PHNUM, AT_ENTRY, AT_BASE,
         * AT_PAGESZ, AT_NULL */
        auxv = (Elf64_auxv_t *)kmalloc(7 * sizeof(Elf64_auxv_t));
        if (!auxv)
        {
            ret = -ENOMEM;
            goto done;
        }
        Elf64_auxv_t *auxvent = auxv;

        /* Add all the necessary entries */
        auxvent->a_type = AT_PHDR;
        auxvent->a_un.a_ptr = pht;
        auxvent++;

        auxvent->a_type = AT_PHENT;
        auxvent->a_un.a_val = header.e_phentsize;
        auxvent++;

        auxvent->a_type = AT_PHNUM;
        auxvent->a_un.a_val = header.e_phnum;
        auxvent++;

        auxvent->a_type = AT_ENTRY;
        auxvent->a_un.a_ptr = (void *)header.e_entry;
        auxvent++;

        auxvent->a_type = AT_BASE;
        auxvent->a_un.a_ptr = interpbase;
        auxvent++;

        auxvent->a_type = AT_PAGESZ;
        auxvent->a_un.a_val = PAGE_SIZE;
        auxvent++;

        auxvent->a_type = AT_NULL;
    }
    else
    {
        /* Just put AT_NULL (we don't really need this at all) */
        auxv = (Elf64_auxv_t *)kmalloc(sizeof(Elf64_auxv_t));
        if (!auxv)
        {
            ret = -ENOMEM;
            goto done;
        }
        auxv->a_type = AT_NULL;
    }

    /* Allocate stack at the top of the address space */
    uint64_t stack_lopage = (uint64_t)vmmap_find_range(
        map, (DEFAULT_STACK_SIZE / PAGE_SIZE) + 1, VMMAP_DIR_HILO);
    if (stack_lopage == ~0UL)
    {
        ret = -ENOMEM;
        goto done;
    }
    ret =
        vmmap_map(map, NULL, stack_lopage, (DEFAULT_STACK_SIZE / PAGE_SIZE) + 1,
                  PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_FIXED, 0, 0, NULL);
    KASSERT(0 == ret);
    dbg(DBG_ELF, "Mapped Stack at low addr 0x%p, size %#lx\n",
        PN_TO_ADDR(stack_lopage), DEFAULT_STACK_SIZE + PAGE_SIZE);

    /* Calculate size needed on user stack for arguments */
    size_t argc, envc, auxc;
    size_t argsize =
        _elf64_calc_argsize(argv, envp, auxv, phtsize, &argc, &envc, &auxc);
    /* Make sure it fits on the stack */
    if (argsize >= DEFAULT_STACK_SIZE)
    {
        ret = -E2BIG;
        goto done;
    }
    /* Allocate kernel buffer for temporarily storing arguments */
    argbuf = (char *)kmalloc(argsize);
    if (!argbuf)
    {
        ret = -ENOMEM;
        goto done;
    }
    /* Calculate where in user space we start putting the args. */
    // the args go at the beginning (top) of the stack
    void *arglow =
        (char *)PN_TO_ADDR(stack_lopage) +
        (uint64_t)(
            ((uint64_t)PN_TO_ADDR((DEFAULT_STACK_SIZE / PAGE_SIZE) + 1)) -
            argsize);

    /* Copy everything into the user address space, modifying addresses in
     * argv, envp, and auxv to be user addresses as we go. */
    _elf64_load_args(map, arglow, argsize, argbuf, argv, envp, auxv, argc, envc,
                     auxc, phtsize);

    dbg(DBG_ELF,
        "Past the point of no return. Swapping to map at 0x%p, setting brk to "
        "0x%p\n",
        map, proghigh);
    /* the final threshold / What warm unspoken secrets will we learn? / Beyond
     * the point of no return ... */

    /* Give the process the new mappings. */
    vmmap_destroy(&curproc->p_vmmap);
    map->vmm_proc = curproc;
    curproc->p_vmmap = map;
    map = NULL; /* So it doesn't get cleaned up at the end */

    /* Flush the process pagetables and TLB */
    pt_unmap_range(curproc->p_pml4, USER_MEM_LOW, USER_MEM_HIGH);
    tlb_flush_all();

    /* Set the process break and starting break (immediately after the mapped-in
     * text/data/bss from the executable) */
    curproc->p_brk = proghigh;
    curproc->p_start_brk = proghigh;

    strncpy(curproc->p_name, filename, PROC_NAME_LEN);

    /* Tell the caller the correct stack pointer and instruction
     * pointer to begin execution in user space */
    *rip = (uint64_t)entry;
    *rsp = ((uint64_t)arglow) -
           8; /* Space on the user stack for the (garbage) return address */
    /* Note that the return address will be fixed by the userland entry code,
     * whether in static or dynamic */

    /* And we're done */
    ret = 0;

// https://www.youtube.com/watch?v=PJhXVg2QisM
done:
    fput(&file);
    if (map)
    {
        vmmap_destroy(&map);
    }
    if (pht)
    {
        kfree(pht);
    }
    if (interpname)
    {
        kfree(interpname);
    }
    if (interpfd >= 0)
    {
        do_close((int)interpfd);
    }
    if (interpfile)
    {
        fput(&interpfile);
    }
    if (interppht)
    {
        kfree(interppht);
    }
    if (auxv)
    {
        kfree(auxv);
    }
    if (argbuf)
    {
        kfree(argbuf);
    }
    return ret;
}

void elf64_init(void) { binfmt_add("ELF64", _elf64_load); }
