/*
 *  File: ldreloc_x86_64.c
 *  Date: Mar 26 2019
 *  Acct: Sandy Harvie (charvie)
 *  Desc: x86-64 ELF dynamic linker
 */

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/types.h"

#include "ldresolve.h"
#include "ldtypes.h"
#include "ldutil.h"

extern void _ld_bind(void);

ldinit_t _ldloadrtld(int argc, char **argv, char **envp, Elf64_auxv_t *auxv)
{
    /* Find our own base address in the auxiliary vector */
    Elf64_Addr base = 0;
    for (size_t i = 0; auxv[i].a_type != AT_NULL; i++)
    {
        if (auxv[i].a_type == AT_BASE)
        {
            base = (Elf64_Addr)auxv[i].a_un.a_val;
            break;
        }
    }
    if (!base)
        exit(1);

    /* Make sure we are ourselves */
    Elf64_Ehdr *hdr = (Elf64_Ehdr *)base;
    if (hdr->e_ident[EI_MAG0] != ELFMAG0 || hdr->e_ident[EI_MAG1] != ELFMAG1 ||
        hdr->e_ident[EI_MAG2] != ELFMAG2 || hdr->e_ident[EI_MAG3] != ELFMAG3)
    {
        exit(1);
    }

    /* Find our program header */
    Elf64_Phdr *phdr = (Elf64_Phdr *)(base + hdr->e_phoff);

    /* Find our dynamic segment */
    while (phdr->p_type != PT_DYNAMIC)
    {
        phdr++;
    }

    /* Find relocation entries in the dynamic segment */
    size_t d_reloff = 0;
    size_t d_relcount = 0;
    Elf64_Dyn *dyn = (Elf64_Dyn *)(base + phdr->p_vaddr);
    for (; dyn->d_tag != DT_NULL; dyn++)
    {
        if (dyn->d_tag == DT_RELA)
        {
            d_reloff = dyn->d_un.d_ptr;
        }
        else if (dyn->d_tag == DT_RELACOUNT)
        {
            d_relcount = dyn->d_un.d_val;
        }
    }

    /* Relocate ourselves */
    Elf64_Rela *rel = (Elf64_Rela *)(base + d_reloff);
    for (size_t i = 0; i < d_relcount; i++)
    {
        size_t type = ELF64_R_TYPE(rel[i].r_info);
        if (type == R_X86_64_RELATIVE)
        {
            Elf64_Addr *addr = (Elf64_Addr *)(base + rel[i].r_offset);
            *addr = base + rel[i].r_addend;
        }
        else
        {
            fprintf(stderr, "_ldloadrtld: unsupported relocation type: %lu\n",
                    type);
            exit(1);
        }
    }

    /* Relocate the executable */
    return _ldstart(envp, auxv);
}

void _ldrelocobj(module_t *module)
{
    Elf64_Addr base = module->base;

    for (size_t i = 0; i < module->nreloc; i++)
    {
        Elf64_Rela rel = module->reloc[i];

        uint64_t sym = ELF64_R_SYM(rel.r_info);
        uint64_t type = ELF64_R_TYPE(rel.r_info);
        const char *name = module->dynstr + module->dynsym[sym].st_name;
        void *addr = (void *)(base + rel.r_offset);

        Elf64_Word size;
        ldsym_t symbol;
        switch (type)
        {
        case R_X86_64_RELATIVE:
            *(Elf64_Addr *)addr = base + rel.r_addend;
            break;
        case R_X86_64_COPY:
            symbol = _ldresolve(module, name, -1, &size, 1);
            memcpy(addr, symbol, size);
            break;
        case R_X86_64_JUMP_SLOT:
        case R_X86_64_GLOB_DAT:
            symbol = _ldresolve(module, name, -1, 0, 0);
            *(Elf64_Addr *)addr = (Elf64_Addr)symbol;
            break;
        case R_X86_64_32:
            symbol = _ldresolve(module, name, -1, 0, 0);
            *(Elf64_Addr *)addr = (Elf64_Addr)symbol + rel.r_addend;
            break;
        case R_X86_64_PC32:
            symbol = _ldresolve(module, name, -1, 0, 0);
            *(Elf64_Addr *)addr =
                (Elf64_Addr)symbol + rel.r_addend - (Elf64_Addr)addr;
            break;
        default:
            fprintf(stderr,
                    "_ldrelocobj: unsupported relocation type: %lu\n",
                    type);
            exit(1);
        }
    }
}

void _ldrelocplt(module_t *module)
{
    for (size_t i = 0; i < module->npltreloc; i++)
    {
        Elf64_Rela rel = module->pltreloc[i];

        uint64_t type = ELF64_R_TYPE(rel.r_info);
        if (type != R_X86_64_JUMP_SLOT)
        {
            fprintf(stderr, "_ldrelocplt: unsupported relocation type: %lu\n",
                    type);
            exit(1);
        }

        *(Elf64_Addr *)(module->base + rel.r_offset) += module->base;
    }
}

void _ldpltgot_init(module_t *module)
{
    Elf64_Addr *pltbase = module->pltgot;
    pltbase[1] = (Elf64_Addr)module;
    pltbase[2] = (Elf64_Addr)&_ld_bind;
}

void _ldbindnow(module_t *module)
{
    Elf64_Addr base = module->base;

    for (size_t i = 0; i < module->npltreloc; i++)
    {
        Elf64_Rela rel = module->pltreloc[i];

        uint64_t sym = ELF64_R_SYM(rel.r_info);
        uint64_t type = ELF64_R_TYPE(rel.r_info);
        const char *name = module->dynstr + module->dynsym[sym].st_name;
        void *addr = (void *)(base + rel.r_offset);

        if (type != R_X86_64_JUMP_SLOT)
        {
            fprintf(stderr, "_ldbindnow: unsupported relocation type: %lu\n",
                    type);
            exit(1);
        }

        ldsym_t symbol = _ldresolve(module, name, -1, 0, 0);
        *(Elf64_Addr *)addr = (Elf64_Addr)symbol;
    }
}
