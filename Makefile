ifeq ($(BUILD),debug)
CFLAGS = -Werror -std=c99 -Wall -Wextra -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -Wconversion -Wno-error=sign-conversion -fsanitize-undefined-trap-on-error -fno-diagnostics-color -g3
else
CFLAGS = -Werror -std=c99 -Wall -Wextra -fno-diagnostics-color -O2
endif

%.s: %.c
	$(CC) -O2 -Werror -Wall -Wextra -fno-builtin -std=gnu99 -mgeneral-regs-only -S -fpie $<

%_strip.c: %.strip
	python asmify.py $< > $@

%_strip: %_strip.c
	$(CC) -O2 -Werror -Wall -Wextra -std=gnu99 -fno-builtin -fpie -mgeneral-regs-only -o $@ $< -nostdlib
	objdump -d -S $@ > $@.asm
	objcopy -O binary --only-section=.text $@ $@.bin

test: test.c
elf_injector: elf_injector.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	-rm *.s
	-rm *_strip.c
	-rm *_strip.asm
	-rm *_strip.bin
	-rm *_strip
	-rm elf_injector
	-rm test
