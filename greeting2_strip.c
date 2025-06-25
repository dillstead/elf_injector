#include <unistd.h>
#include <elf.h>
#include <stddef.h>
#include <syscall.h>
#include "inject_info.h"
#include "fc.h"

#define countof(a)  (sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)

#define SYSCALL3(n, a, b, c) \
    syscall3(n,(long)(a),(long)(b),(long)(c))

static long syscall3(long n, long a, long b, long c)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    register long r2 asm("r2") = c;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0), "r"(r1), "r"(r2)
        : "r9", "r12", "r14", "memory"
    );
    return ret;
}

// _start offset: 0
void _start(int argc, char **argv, char **env, Elf32_auxv_t *aux, struct inject_info *ii)
{
    (void) argc;
    (void) argv;
    (void) env;
    (void) aux;
    (void) ii;

    char greeting[lengthof("Hello World\n")];
    FILL_CHARS(greeting, 'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd', '\n');
    SYSCALL3(SYS_write, 1, greeting, sizeof(greeting));
}

