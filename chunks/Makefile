CFLAGS = -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fno-diagnostics-color -fpie -O2

%.s: %.c
	$(CC) $(CFLAGS) -S $<

%_strip.c: %.strip
	python ../tools/asmify.py $< > $@

%_strip: %_strip.c
	$(CC) $(CFLAGS) -o $@ $<
	objdump -d -S $@ > $@.asm
	objcopy -O binary --only-section=.text $@ $@.bin

.PHONY: all clean

all: greeting_strip startup_dump_strip copy_chunk_strip

clean: 
	rm -f *_strip
	rm -f *_strip.asm
	rm -f *_strip.bin

