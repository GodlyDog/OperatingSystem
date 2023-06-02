/* Assembler macros for x86-64.
   Copyright (C) 2018-2019 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _ASM_H
#define _ASM_H 1

/* ELF uses byte-counts for .align, most others use log2 of count of bytes.  */
#define ALIGNARG(log2) 1 << log2
#define ASM_SIZE_DIRECTIVE(name) .size name, .- name;

/* Define an entry point visible from C.  */
#define ENTRY(name)        \
    .globl name;           \
    .type name, @function; \
    .align ALIGNARG(4);    \
    name## :.cfi_startproc;

#define END(name) \
    .cfi_endproc; \
    ASM_SIZE_DIRECTIVE(name)

#endif /* _ASM_H */
