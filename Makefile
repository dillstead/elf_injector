CFLAGS =  -O2 -Werror -std=gnu99 -Wall -Wextra
#LDFLAGS = -nostdlib -static -lgcc
LDFLAGS = -nostdlib -static

all: greeting.s

greeting.s:
	$(CC) $(CFLAGS) -S greeting.c

greeting_strip.c: greeting_strip.s
	python asmify.py ./greeting_strip.s > ./greeting_strip.c

greeting_strip: greeting_strip.c
	$(CC) $(CFLAGS) -o greeting_strip greeting_strip.c $(LDFLAGS)
	objdump -d -S greeting_strip > greeting_strip.asm
	objcopy greeting_strip -O binary greeting_strip.bin
	readelf -h -l -S greeting_strip > greeting_strip.re

clean:
	-rm greeting_strip
	-rm greeting.s
	-rm greeting_strip.c
	-rm greeting_strip.asm
	-rm greeting_strip.re
	-rm greeting_strip.bin







