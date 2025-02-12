// Thunk that loads and runs injected code and returns control
// to the original entry point when it's finished. 
#include <unistd.h>
#include <stddef.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <syscall.h>

typedef __UINT8_TYPE__   u8;
typedef __INT32_TYPE__   b32;
typedef __INT32_TYPE__   i32;
typedef __UINT64_TYPE__  u64;
typedef __INT64_TYPE__   i64;
typedef __PTRDIFF_TYPE__ size;
typedef __UINTPTR_TYPE__ uptr;
typedef char             byte;

#define PGBITS  12                         
#define PGSIZE  (1 << PGBITS)              
#define SYSCALL1(n, a)                \
    syscall1(n,(long)(a))
#define SYSCALL2(n, a, b)             \
    syscall2(n,(long)(a),(long)(b))
#define SYSCALL6(n, a, b, c, d, e, f) \
    syscall6(n,(long)(a),(long)(b), (long)(c), (long)(d), (long)(e), (long)(f))

static long syscall1(long n, long a)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0)
        : "r9", "r12", "r14", "memory"
    );
    return ret;
}

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
        : "r9", "r12", "r14", "memory"
    );
    return ret;
}

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

// These values will be patched by elf_injector at injection time.
static volatile size code_off = 0x00000001;        // offset 204
static volatile size code_len = 0x00000002;        // offset 208
static volatile size code_entry_off = 0x00000003;  // offset 212
static volatile size host_entry = 0x00000004;      // offset 216

// _start offset: 0
void _start(int argc, char **argv)
{
    int fd = -1;
    if ((fd = (int) SYSCALL2(SYS_open, argv[0], O_RDONLY)) == -1)
    {
        goto cleanup;
    }

    u8 *code;
    if ((code = (u8 *) SYSCALL6(SYS_mmap2, NULL, code_len,
                                PROT_READ | PROT_EXEC,
                                MAP_PRIVATE, fd, code_off)) == MAP_FAILED)
    {
        goto cleanup;
    }
    
    void (*code_entry)(int, char **)
          = (void (*)(int, char **)) (code + code_entry_off);
    code_entry(argc, argv);
    SYSCALL2(SYS_munmap, code, code_len);
cleanup:
    SYSCALL1(SYS_close, fd);
    ((void (*)(void)) host_entry)();
}
