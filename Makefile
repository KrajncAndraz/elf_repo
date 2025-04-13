CC=gcc
CFLAGS=-Wall -Wextra -Werror -Wpedantic

all: build run

build: build_elf_changer build_hello

build_elf_changer:
	@$(CC) $(CFLAGS) elf_changer.c elflib.c -o elf_changer

build_hello:
	@$(CC) $(CFLAGS) hello.c -o hello

run:
	./elf_changer -h hello
	./elf_changer -l hello
	./elf_changer -c hello g1 g2

clean:
	-rm -f elf_changer hello *.o
