#include <unistd.h>
#include <stddef.h>
#include <syscall.h>

typedef __UINT8_TYPE__   u8;
typedef __INT8_TYPE__    i8;
typedef __UINT16_TYPE__  u16;
typedef __INT16_TYPE__   i16;
typedef __UINT32_TYPE__  u32;
typedef __INT32_TYPE__   i32;
typedef __UINT64_TYPE__  u64;
typedef __INT64_TYPE__   i64;
typedef __PTRDIFF_TYPE__ size;
typedef __SIZE_TYPE__    usize;
typedef __UINTPTR_TYPE__ uptr;
typedef __INTPTR_TYPE__  iptr;
typedef char             byte;

#define sizeof(x)   (size) sizeof(x)
#define countof(a)  (size)(sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)

#define SYSCALL3(n, a, b, c) \
    syscall3(n,(long)(a),(long)(b),(long)(c))

static i32 slen(const char *str)
{
    if (!str)
    {
        return 0;
    }
    
    i32 len = 0;
    while (*str++)
    {
        len++;
    }
    return len;
}

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

// _start offset: XXX
void _start(int argc, char **argv)
{
    full_write((const u8 *) "Hello World!\n", lengthof("Hello World!\n"));
    for (int i = 0; i < argc; i++)
    {
        SYSCALL3(SYS_write, 1, argv[i], slen(argv[i]));
        SYSCALL3(SYS_write, 1, "\n", lengthof("\n"));
    }
}
