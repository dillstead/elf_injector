CFLAGS =  -O2 -Werror -std=gnu99 -Wall -Wextra
LDFLAGS = -nostdlib -static -lgcc

greeting.s:
	$(CC) $(CFLAGS) -S -fpie -mgeneral-regs-only greeting.c

greeting_strip.c: greeting_strip.s
	python asmify.py ./greeting_strip.s > ./greeting_strip.c

greeting_strip: greeting_strip.c
	$(CC) $(CFLAGS) -o greeting_strip greeting_strip.c $(LDFLAGS)
	objdump -d -S greeting_strip > greeting_strip.asm
	objcopy -O binary --only-section=.text greeting_strip greeting_strip.bin
	readelf -h -l -S greeting_strip > greeting_strip.re

test: test.c
	$(CC) $(CFLAGS) -o test test.c
	objdump -d -S test > test.asm

clean:
	-rm greeting_strip
	-rm greeting.s
	-rm greeting_strip.c
	-rm greeting_strip.asm
	-rm greeting_strip.re
	-rm greeting_strip.bin







