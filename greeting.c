#include <unistd.h>
#include <stddef.h>
#include <syscall.h>

#define sizeof(x)   (ptrdiff_t) sizeof(x)
#define countof(a)  (size)(sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)

#define SYSCALL1(n, a)                          \
    syscall6(n,(long)(a),0,0,0,0,0)
#define SYSCALL2(n, a, b) \
    syscall6(n,(long)(a),(long)(b),0,0,0,0)
#define SYSCALL3(n, a, b, c) \
    syscall6(n,(long)(a),(long)(b),(long)(c),0,0,0)
#define SYSCALL4(n, a, b, c, d) \
    syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),0,0)
#define SYSCALL5(n, a, b, c, d, e) \
    syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),(long)(e),0)
#define SYSCALL6(n, a, b, c, d, e, f) \
    syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),(long)(e),(long)(f))

typedef __UINT8_TYPE__   u8;
typedef __INT32_TYPE__   b32;
typedef __INT32_TYPE__   i32;
typedef __UINT64_TYPE__  u64;
typedef __INT64_TYPE__   i64;
typedef __PTRDIFF_TYPE__ size;
typedef __UINTPTR_TYPE__ uptr;
typedef char             byte;

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

// https://man7.org/linux/man-pages/man2/syscall.2.html
static long syscall6(long n, long a, long b, long c, long d, long e, long f)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    register long r2 asm("r2") = c;
    register long r3 asm("r3") = d;
    register long r4 asm("r4") = e;
    register long r5 asm("r5") = f;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4), "r"(r5)
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

void _start(int argc, char **argv)
{
    full_write((const u8 *) "Hello World!\n", lengthof("Hello World!\n"));
    for (int i = 0; i < argc; i++)
    {
        full_write((const u8 *) argv[i], slen(argv[i]));
        full_write((const u8 *) "\n", lengthof("\n"));
    }
    // Manually patch in call to host entry point here.
    // Entry offset to start running: 0/0x0
    // Host entry offset from beginning of file: 326/0x146
}
