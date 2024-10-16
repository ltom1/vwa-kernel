NAME=HELLO.ELF

CC=gcc
AS=nasm
LK=ld

IMG=../../../os.img
MNT=/mnt
PROG=/mnt/PROG/
LOOP=/dev/loop0
LOOPP1=/dev/loop0p1

ASM_SRC=$(shell find . -type f -name '*.asm')
ASM_OBJ=$(patsubst %.asm,%.o,$(ASM_SRC))

C_SRC=$(shell find . -type f -name '*.c')
C_OBJ=$(patsubst %.c,%.o,$(C_SRC))

HEADERS=$(shell find ../../../src/ -type f -name '*.h')


all: clean $(NAME) copy

$(NAME): $(C_OBJ) $(ASM_OBJ) $(HEADERS)
	$(LK) -o $@ $(C_OBJ) $(ASM_OBJ)


%.o: %.asm
	$(AS) -g3 -F dwarf -f elf64 $< -o $@

%.o: %.c
	$(CC) -mgeneral-regs-only -masm=intel -Wall -I../../include -Iinclude -mcmodel=large -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -ffreestanding -fno-pie -fno-stack-protector -g -c $< -o $@


copy: $(NAME)
	sudo losetup -P $(LOOP) $(IMG)
	sudo mount $(LOOPP1) $(MNT)
	sudo cp $< $(PROG)
	sudo umount $(LOOPP1)
	sudo losetup -d $(LOOP)


clean:
	rm -f -- *.ELF
	rm -f -- *.o
	rm -f -- */*.o
	rm -f -- */*/*.o
	rm -f -- */*/*/*.o
	rm -f -- */*/*/*/*.o
