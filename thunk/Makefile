CFLAGS = -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fno-diagnostics-color -fpie -O2

thunk_array: thunk_strip
	python ../tools/arrayify.py $<.bin _thunk 4096

%.s: %.c
	$(CC) $(CFLAGS) -S $<

%_strip.c: %.strip
	python ../tools/asmify.py $< > $@

%_strip: %_strip.c
	$(CC) $(CFLAGS) -o $@ $<
	objdump -d -S $@ > $@.asm
	objcopy -O binary --only-section=.text $@ $@.bin

.PHONY: all clean

all: thunk_strip

clean: 
	-rm *_strip
	-rm *_strip.asm
	-rm *_strip.bin
