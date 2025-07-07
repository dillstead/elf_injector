CC = gcc
CFLAGS = -Werror -std=gnu99 -fno-builtin -Wall -Wextra -fno-diagnostics-color -O2
VPATH = src

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

elf_injector: elf_injector.o
	$(CC) -nostdlib $< -o $@

test: test.o
	$(CC) $< -o $@

.PHONY: all clean

all: elf_injector test

clean:
	-rm *.o
	-rm elf_injector
	-rm test
