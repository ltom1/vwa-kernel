AS=nasm
CC=gcc
LK=ld
DBG=gdb

VM=qemu-system-x86_64

OBJCPY=objcopy
TERM=alacritty
WORKING_DIR=$(shell pwd)

IMG=os.img

MNT=/mnt
PROG=/mnt/PROG/
LOOP=/dev/loop0
LOOPP1=/dev/loop0p1
KERNEL_ASM_SRC=$(shell find . -type f -name '*.asm' -not -path "./src/programs/*")
KERNEL_ASM_OBJ=$(patsubst %.asm,%.o,$(KERNEL_ASM_SRC))

KERNEL_C_SRC=$(shell find . -type f -name "*.c" -not -path "./src/programs/*")
KERNEL_C_OBJ=$(patsubst %.c,%.o,$(KERNEL_C_SRC))

PROGRAMS=$(shell find src/programs -type f -name '*.ELF')

KERNEL_HEADERS=$(shell find src/ -type f -name '*.h')

LINKER_SCRIPT=src/linker.ld



all: clean run

run: $(IMG)
	$(VM) -d int,cpu_reset,guest_errors,page -no-reboot -debugcon stdio -hda $<


debug: $(IMG)
	$(TERM) --working-directory $(WORKING_DIR) -e $(VM) -s -S -d int,cpu_reset,guest_errors,page -no-reboot -debugcon stdio -hda $< &
	$(DBG) KERNEL.ELF \
        -ex 'target remote localhost:1234' \
        -ex 'layout src' \
        -ex 'layout regs' \
        -ex 'break kmain' \
        -ex 'continue'


$(IMG): KERNEL.BIN
	sudo losetup -P $(LOOP) $@
	sudo mount $(LOOPP1) $(MNT)
	sudo cp $< $(MNT)
	sudo umount $(LOOPP1)
	sudo losetup -d $(LOOP)

src/: programs/PROG.ELF
	sudo losetup -P $(LOOP) $(IMG)
	sudo mount $(LOOPP1) $(MNT)
	sudo cp $< $(MNT)
	sudo umount $(LOOPP1)
	sudo losetup -d $(LOOP)


KERNEL.BIN: KERNEL.ELF
	$(OBJCPY) -O binary $< $@

KERNEL.ELF: $(KERNEL_HEADERS) $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ) $(LINKER_SCRIPT)
	$(LK) -o $@ -T $(LINKER_SCRIPT) $(KERNEL_ASM_OBJ) $(KERNEL_C_OBJ)


%.o: %.asm
	$(AS) -g3 -F dwarf -f elf64 $< -o $@

%.o: %.c
	$(CC) -mgeneral-regs-only -masm=intel -Wall -Isrc/include -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding -fno-pie -fno-stack-protector -g -c $< -o $@


clean:
	rm -f -- *.BIN
	rm -f -- *.ELF
	rm -f -- *.mem
	rm -f -- *.o
	rm -f -- */*.o
	rm -f -- */*/*.o
	rm -f -- */*/*/*.o
	rm -f -- */*/*/*/*.o
