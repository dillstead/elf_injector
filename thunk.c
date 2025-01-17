//#include <unistd.h>
//#include <stddef.h>
//#include <syscall.h>

typedef __UINT8_TYPE__   u8;
typedef __INT32_TYPE__   b32;
typedef __INT32_TYPE__   i32;
typedef __UINT64_TYPE__  u64;
typedef __INT64_TYPE__   i64;
typedef __PTRDIFF_TYPE__ size;
typedef __UINTPTR_TYPE__ uptr;
typedef char             byte;

#define SYSCALL1(n, a)                          \
    syscall6(n,(long)(a),0,0,0,0,0)
#define SYSCALL2(n, a, b) \
    syscall6(n,(long)(a),(long)(b),0,0,0,0)
#define SYSCALL5(n, a, b, c, d, e) \
    syscall6(n,(long)(a),(long)(b),(long)(c),(long)(d),(long)(e),0)

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
        : "r"(r7), "r"(r0), "r"(r1),
        : "r9", "r12", "r14", "memory"
    );
    return ret;
}

static long syscall5(long n, long a, long b, long c, long d, long e)
{
    register long ret asm("r0");
    register long r7 asm("r7") = n;
    register long r0 asm("r0") = a;
    register long r1 asm("r1") = b;
    register long r2 asm("r2") = c;
    register long r3 asm("r3") = d;
    register long r4 asm("r4") = e;
    __asm volatile (
        "swi #0\n"
        : "=r"(ret)
        : "r"(r7), "r"(r0), "r"(r1), "r"(r2), "r"(r3), "r"(r4)
        : "r9", "r12", "r14", "memory"
    );
    return ret;
}

#define PAGE_SZ 4096

// These values will be patched by elf_xinjector at injection time.
static size code_offset = -1;
static size code_len = -2;
static size code_entry_offset = -3;

void _start(int argc, char **argv)
{
    int fd;
    if ((fd = (int) SYSCALL2(SYS_open, argv[0], O_RDONLY)) == -1)
    {
        goto done;
    }

    // Must be a multiple of page size.
    size code_pg_offset = code_offset & ~(PAGE_SZ - 1);
    aize code_pg_len = code_offset - code_poffset + code_len;
    u8 *code;
    if ((code = (u8 *) SYSCALL5(SYS_mmap, NULL, code_plen,
                                PROT_READ | PROT_EXEC,
                                MAP_PRIVATE, fd, code_poffset)) == MAP_FAILED)
    {
        goto done;
    }
    void (*code_entry)(int, char **)
        = void (*)(int, char **) (code + (code_offset - code_poffset) + code_entry_offset);
    code_entry(argc, argv);
    SYSCALL2(SYS_munmap, code, code_plen);
done:
    SYSCALL1(SYS_close, fd);
    // Manually patch in call to host entry point here.
    // Entry offset to start running: 0/0x****
    // Host entry offset from beginning of file: 0/0x****
}
