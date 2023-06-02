/*
 *  File: ldtypes.h
 *  Date: 12 April 1998
 *  Acct: David Powell (dep)
 *  Desc:
 *
 *
 *  Acct: Sandy Harvie (charvie)
 *  Date: 26 March 2019
 *  Desc: Modified for x86-64
 */

#ifndef _ldtypes_h_
#define _ldtypes_h_

#include "elf.h"

#define LD_ERR_EXIT 13

typedef Elf64_auxv_t auxv_t; /* linux is funky */

typedef int (*ldfunc_t)();
typedef void *ldsym_t;
typedef void (*ldinit_t)(int argc, char **argv, char **envp, auxv_t *auxv);

typedef struct ldenv_t
{
    int ld_bind_now;
    int ld_debug;
    const char *ld_preload;
    const char *ld_library_path;
} ldenv_t;

extern ldenv_t _ldenv;

typedef struct module
{
    char *name;    /* the filename                 */
    char *runpath; /* the run path to use          */

    unsigned long base; /* base address of module       */
    Elf64_Word *hash;   /* the module's hash table      */
    Elf64_Sym *dynsym;  /* the dynamic symbol table     */
    char *dynstr;       /* the dynamic string table     */

    ldfunc_t init; /* module initialization fcn.   */
    ldfunc_t fini; /* module shutdown fcn.         */

    Elf64_Rela *pltreloc; /* PLT relocations              */
    Elf64_Rela *reloc;    /* normal relocations           */

    size_t nreloc;    /* number of relocation entries */
    size_t npltreloc; /* number of relocation entries */

    struct module *next;  /* the next module in the chain */
    struct module *first; /* the first module             */
    Elf64_Addr *pltgot;   /* base of plt                  */
} module_t;

#endif /* _ldtypes.h_ */
