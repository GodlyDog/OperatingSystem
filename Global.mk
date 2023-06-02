# To build a 32-bit/64-bit weenix, use `make all ARCH=i686/x86-64` or `make all_32/64`
#
# We set ARCH to x86-64 by default so that `make clean` doesn't cause an error 
# in the conditional make statements that require ARCH to be set
ARCH      := x86-64
CC        := gcc
LD        := ld
AR        := ar
PYTHON    := python2
CSCOPE    := cscope
MKRESCUE  := grub-mkrescue

cflags.x86-64	:= -march=x86-64 -m64 -mno-red-zone -mcmodel=large -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2 -mno-sse4 -mno-sse4a -mno-3dnow -mno-avx -mno-avx2
cflags.common	:= -fno-pie -ffreestanding -fno-builtin -nostdinc -std=c99 -g3 -gdwarf-3 -fno-stack-protector -fsigned-char -Iinclude
cflags.warnings	:= -Wall -Wredundant-decls -Wundef -Wpointer-arith -Wfloat-equal -Wnested-externs -Wvla -Winline -Wextra -Wno-unused-parameter -Wno-unused-function -Wno-unused-variable -Wno-attributes

# We need the `-no-pie` flag as well if we are compiling on GCC 5 or higher.
GCCVERSIONGTEQ5 := $(shell expr `gcc -dumpversion | cut -f1 -d.` \>= 5)
ifeq "$(GCCVERSIONGTEQ5)" "1"
    CFLAGS += -no-pie
endif


CFLAGS			+= ${cflags.${ARCH}} ${cflags.common} ${cflags.warnings} 
ASFLAGS			:= -D__ASSEMBLY__

###

include ../Config.mk

CFLAGS	+= $(foreach bool,$(COMPILE_CONFIG_BOOLS), \
				$(if $(findstring 1,$($(bool))),-D__$(bool)__=$(strip $($(bool)))))

CFLAGS	+= $(foreach def,$(COMPILE_CONFIG_DEFS), \
				$(if $($(def)),-D__$(def)__=$(strip $($(def))),))