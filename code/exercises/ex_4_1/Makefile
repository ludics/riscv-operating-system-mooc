CC=riscv64-linux-gnu-gcc

all:
	$(CC) hello.c -I/usr/riscv64-linux-gnu/include -static -march=rv64id -g -Wall -o hello.elf

run:
	qemu-riscv64 hello.elf

clean:
	rm -rf *.o *.bin *.elf
