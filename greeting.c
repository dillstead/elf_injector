#include <unistd.h>
#include <stddef.h>
#include <syscall.h>

typedef __UINT8_TYPE__   u8;
typedef __INT32_TYPE__   b32;
typedef __INT32_TYPE__   i32;
typedef __UINT64_TYPE__  u64;
typedef __INT64_TYPE__   i64;
typedef __PTRDIFF_TYPE__ size;
typedef __UINTPTR_TYPE__ uptr;
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

static b32 full_write(const u8 *s, i32 len)
{
    for (i32 off = 0; off < len;)
    {
        i32 r = (i32) SYSCALL3(SYS_write, 1, s + off, len - off);
        if (r < 1)
        {
            return 0;
        }
        off += r;
    }
    return 1;
}

// _start offset: 20
void _start(int argc, char **argv)
{
    full_write((const u8 *) "Hello World!\n", lengthof("Hello World!\n"));
    for (int i = 0; i < argc; i++)
    {
        full_write((const u8 *) argv[i], slen(argv[i]));
        full_write((const u8 *) "\n", lengthof("\n"));
    }
}
