// Thunk that loads and runs injected code and returns control
// to the original entry point when it's finished. 
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <syscall.h>
#include <elf.h>
#include <linux/auxvec.h>
#include "../inc/inject_info.h"

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
