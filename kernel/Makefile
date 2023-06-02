LDFLAGS   := --build-id=none -z max-page-size=0x1000 -n

include ../Global.mk

CFLAGS    += -D__KERNEL__

###

HEAD      := $(wildcard include/*/*.h include/*/*/*.h)
SRCDIR    := boot entry main util drivers drivers/disk drivers/tty mm proc fs/ramfs fs/s5fs fs vm api test test/kshell test/vfstest

SRC       := $(foreach dr, $(SRCDIR), $(wildcard $(dr)/*.[cS]))

OBJS      := $(addsuffix .o,$(basename $(SRC)))
ASM_FILES :=
SCRIPTS   := $(foreach dr, $(SRCDIR), $(wildcard $(dr)/*.gdb $(dr)/*.py))

BSYMBOLS  := symbols.dbg
KERNEL    := kernel.bin
IMAGE     := weenix.img
ISO_IMAGE := weenix.iso
GDBCOMM   := gdb-commands

.PHONY: all cscope clean

%.a:
	touch $@

all: $(ISO_IMAGE) $(GDBCOMM) $(ASM_FILES) $(BSYMBOLS)

$(KERNEL) : $(OBJS)
	@ echo "  Linking for \"kernel/$@\"..."
	@ # entry.o included from link.ld. boot/boot-{32,64}.S must be the first file so that the multiboot header is close enough to the front.
	@ $(LD) $(LDFLAGS) -T link.ld boot/boot.o $(filter-out boot/boot.o entry/entry.o,$^) -o $@

$(BSYMBOLS): $(KERNEL)
	@ echo "  Generating kernel symbols list..."
	@ readelf -Ws $(KERNEL) | grep -Ev 'SECTION|UND|FILE|Num:|Symbol|^$$' | awk '{printf "0x%s %s\n", $$2, $$8}' > $@

$(ISO_IMAGE): $(KERNEL)
	@ echo "  Creating \"kernel/$@\" from floppy disk image..."
	@ rm -rf .iso
	@ mkdir -p .iso/boot/grub
	@ ln -f $< .iso/boot/$< || cp -f $< .iso/boot/$<
	@ echo "default=0" > .iso/boot/grub/grub.cfg
	@ echo "timeout=0" > .iso/boot/grub/grub.cfg
	@ echo "menuentry \"$@\" {" >> .iso/boot/grub/grub.cfg
	@ echo " echo \"Booting $@ from /boot/$<\" " >> .iso/boot/grub/grub.cfg
	@ echo " echo \"Welcome To 64-bit Weenix!\" " >> .iso/boot/grub/grub.cfg
	@ echo " multiboot2 /boot/$< " >> .iso/boot/grub/grub.cfg
	@ echo " acpi -2 " >> .iso/boot/grub/grub.cfg

	@ echo " boot " >> .iso/boot/grub/grub.cfg
	@ echo " GRUB_GFXMODE=1024x768x32" >> .iso/boot/grub/grub.cfg
	@ echo "}" >> .iso/boot/grub/grub.cfg
	@ $(MKRESCUE) -o $@ ./.iso

$(GDBCOMM): $(SCRIPTS)
	@ echo "  Creating gdb command list..."
	@ > $(GDBCOMM)

%.S: %.c
	@ echo "  Compiling \"kernel/$<\"..." 
	@ $(CC) $(CFLAGS) -S $< -o $@

%.o: %.c
	@ echo "  Compiling \"kernel/$<\"..."
	@ $(CC) -c $(CFLAGS) $< -o $@

%.o: %.S
	@ echo "  Compiling \"kernel/$<\"..."
	@ $(CC) -c $(ASFLAGS) $(CFLAGS) $< -o $@

cscope: $(HEAD) $(SRC)
	@ echo "  Updating cscope symbol cross-reference..."
	@ echo $(HEAD) $(SRC) > cscope.files
	@ $(CSCOPE) -k -b -q -v > /dev/null

FILTER=`echo "DRIVERS $(DRIVERS)\nVFS $(VFS)\nS5FS $(S5FS)\nVM $(VM)" | grep 1 | cut -f1 -d" " | tr "\n" "|"`PROCS
nyi:
	@ echo "  Not yet implemented:"
	@ echo
	@ find . -name \*.c -printf "%P\n" \
| xargs grep -Hn "NOT_YET_IMPLEMENTED" \
| sed -e 's/^\(.*:.*\):.*\"\(.*\): \(.*\)\".*/\2 \1 \3/' \
| grep -E "^($(FILTER))" \
| awk '{printf("%25s %30s() %8s\n", $$2, $$3, $$1)}'

clean:
	@ find . -name "*.o" -type f -delete
	@ rm -f $(OBJS) $(BSYMBOLS) $(KERNEL) $(IMAGE) $(ISO_IMAGE) $(GDBCOMM) */*.gdbcomm cscope*.out cscope.files
	@ rm -rf .iso