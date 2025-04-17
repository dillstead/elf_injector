// gcc -Werror -Wall -Wextra -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -Wconversion -Wno-error=sign-conversion -fno-builtin -std=gnu99 -mgeneral-regs-only -fpie -nostdlib -g3 -o elf_injector_new elf_injector_new.c
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <elf.h>
#include <linux/auxvec.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <syscall.h>
#include <limits.h>

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
typedef unsigned char    bool;
enum
{
   false,
   true
};

#undef st_atime
#undef st_mtime
#undef st_ctime
struct stat64
{
    u64 st_dev;
    u32 __pad1;
    u32 st_ino;
    u32 st_mode;
    u32 st_nlink;
    u32 st_uid;	
    u32 st_gid;	
    u64 st_rdev;
    u64 __pad2;
    i64 st_size;
    i32 st_blksize;
    i32 __pad3;
    i64 st_blocks;
    i32 st_atime;
    u32 st_atime_nsec;
    i32 st_mtime;
    u32 st_mtime_nsec;
    i32 st_ctime;
    u32 st_ctime_nsec;
    u32 __unused4;
    u32 __unused5;
};

#define PGSIZE  (1 << 12)

#define assert(c)   while (!(c)) *(volatile int *)0 = 0
#define sizeof(x)   (size) sizeof(x)
#define countof(a)  (size)(sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)
#define s8(s)      (struct s8){(u8 *)s, lengthof(s)}
#define s8cstr(s)  (struct s8){(u8 *)s, strlen(s)}
#define s8nul       (struct s8){(u8 *)"", 1}

#define SYSCALL1(n, a)                \
    syscall1(n,(long)(a))
#define SYSCALL2(n, a, b)             \
    syscall2(n,(long)(a),(long)(b))
#define SYSCALL3(n, a, b, c)          \
    syscall3(n,(long)(a),(long)(b),(long)(c))
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
        : "r1", "r2", "r3", "r4", "r9", "r12", "memory"
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
        : "r2", "r3", "r4", "r9", "r12", "memory"
    );
    return ret;
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
        : "r3", "r4", "r9", "r12", "memory"
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
        : "r9", "r12", "memory"
    );
    return ret;
}

typedef void *jmp_buf[10];

__attribute__((naked, returns_twice))
static int setjmp(jmp_buf buf)
{
    (void) buf;
    __asm volatile(
        "stm r0, { r4-r11, sp, lr }\n"
       "eor r0, r0\n"
        "mov pc, lr\n"
        );
}

__attribute__((naked, noreturn))
static void longjmp(jmp_buf buf, int ret)
{
    (void)buf;
    (void)ret;
    __asm volatile(
        "ldm r0, { r4-r11, sp, lr }\n"
        "mov r0, r1\n"
        "mov pc, lr\n"
        );
}

static void uidivmod(unsigned int n, unsigned int d, unsigned int *q, unsigned int *r)
{
    *q = 0;
    *r = 0;
    if (d == 0)
    {
        return;
    }

    for (int i = 31; i >= 0; i--)
    {
        if (d <= (n >> i))
        {
            n -= (d << i);
            (*q) |= 0x1U << i;
        }
    }
    *r = n;
}

__attribute__((used))
static int __aeabi_idiv(int n, int d)
{
    int s = (n >> 31) ^ (d >> 31) ? -1 : 1;
    unsigned int _n = (unsigned int) n;
    unsigned int _d = (unsigned int) d;
    if (n < 0)
    {
        _n = -_n;
    }
    if (d < 0)
    {
        _d = -_d;
    }
    unsigned int q;
    unsigned int r;
    uidivmod(_n, _d, &q, &r);
    return s * (int) q;
}

static void *memset(void *s, int c, usize n)
{
    u8 *p = s;
    while (n--) *p++ = (u8) c;
    return s;
}

static void *memcpy(void *d, const void *s, usize n)
{
    u8 *p = d;
    const u8 *q = s;
    while (n--) *p++ = *q++;
    return d;
}

struct arena
{
    u8 *beg;
    u8 *end;
    size sz;
    u8 **commit;
    jmp_buf *ctx;
};

static bool new_arena(struct arena *a, jmp_buf *ctx, size sz)
{
    if (sz <= 0)
    {
        return false;
    }
    
    sz += sizeof(a->commit);
    u8 *beg = (u8 *) SYSCALL6(SYS_mmap2, NULL, sz, PROT_NONE,
                              MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (beg == MAP_FAILED
        || SYSCALL3(SYS_mprotect, beg, PGSIZE, PROT_READ | PROT_WRITE) == -1)
    {
        return false;
    }
    
    a->sz = sz;
    a->beg = beg;
    a->end = a->beg + a->sz;
    a->commit = (u8 **) a->beg;
    *a->commit = a->beg + PGSIZE;
    a->beg += sizeof(a->commit);
    a->ctx = ctx;
    return true;
}

#define new(a, t, n) (t *) alloc(a, sizeof(t), _Alignof(t), n)
static void *alloc(struct arena *a, size sz, size align, size count)
{
    size pad = -(iptr) a->beg & (align - 1);
    size avail = a->end - a->beg - pad;
    if (avail < 0 || count > avail / sz)
    {
        longjmp(*a->ctx, 1);
    }
    
    u8 *p = a->beg + pad;
    a->beg += pad + count * sz;
    if (a->beg >= *a->commit)
    {
        if (SYSCALL3(SYS_mprotect, *a->commit, (usize) (a->beg - *a->commit + 1),
                     PROT_READ | PROT_WRITE) == -1)
        {
            longjmp(*a->ctx, 1);
        }
        *a->commit = (u8 *) ((iptr) (a->beg + (PGSIZE - 1)) & ~(PGSIZE - 1));
    }
    
    return memset(p, 0, (usize) (count * sz));
}

struct s8
{
    u8 *data;
    size len;
};

static struct s8 s8cpy(struct arena *a, struct s8 s)
{
    struct s8 r = s;
    r.data = new(a, u8, s.len);
    if (r.len)
    {
        memcpy(r.data, s.data, (usize) r.len);
    }
    return r;
}

static struct s8 s8cat(struct arena *a, struct s8 head, struct s8 tail)
{
    if (!head.data || head.data + head.len != a->beg)
    {
        head = s8cpy(a, head);
    }
    head.len += s8cpy(a, tail).len;
    return head;
}

static int s8open(struct arena a, struct s8 path, int flags)
{
    struct s8 tpath = s8cat(&a, path, s8nul);
    return (int) SYSCALL2(SYS_open, tpath.data, flags);
}

static int s8unlink(struct arena a, struct s8 path)
{
    struct s8 tpath = s8cat(&a, path, s8nul);
    return SYSCALL1(SYS_unlink, tpath.data);
}

static int s8chmod(struct arena a, struct s8 path, mode_t mode)
{
    struct s8 tpath = s8cat(&a, path, s8nul);
    return SYSCALL2(SYS_chmod, tpath.data, mode);
}

static int s8chown(struct arena a, struct s8 path, uid_t own, gid_t grp)
{
    struct s8 tpath = s8cat(&a, path, s8nul);
    return SYSCALL3(SYS_chown, tpath.data, own, grp);
}

static int s8stat64(struct arena a, struct s8 path, struct stat64 *sbuf)
{
    struct s8 tpath = s8cat(&a, path, s8nul);
    return SYSCALL2(SYS_stat64, tpath.data, sbuf);
}

static size strlen(const char *s)
{
    size len = 0;
    while (*s++) len++;
    return len;
}

static int atoi(const char *s)
{
    const char *beg = s;
    int sn = 1;
    if (*beg == '-')
    {
        sn = -1;
        beg++;
    }
    
    int res = 0;
    while (*beg)
    {
        int dig = *beg - '0';
        if (sn == 1)
        {
            if (res > INT_MAX / 10
                || (res == INT_MAX / 10 && dig > INT_MAX % 10))
            {
                return INT_MAX;
            }
            res *= 10;
            res += dig;
        }
        else
        {
            if (res < INT_MIN / 10
                || (res == INT_MIN / 10 && -dig < INT_MIN % 10))
            {
                return INT_MIN;
            }
            res *= 10;
            res -= dig;
        }
        beg++;
    }
    return res;
}

#define fdbuf(fd, b, cap) &(struct buf) {b, cap, 0, fd, 0}
struct buf
{
    u8 *buf;
    size cap;
    size len;
    int fd;
    int err;
};

static u8 outbuf[128];
static struct buf *stdout = &(struct buf) {outbuf, sizeof(outbuf), 0, 1, 0};

void flush(struct buf *b)
{
    b->err |= b->fd < 0;
    if (!b->err && b->len)
    {
        b->err |= SYSCALL3(SYS_write, b->fd, b->buf, b->len) < b->len;
        b->len = 0;
    }
}

static void append(struct buf *b, u8 *src, size len)
{
    u8 *end = src + len;
    while (!b->err && src < end)
    {
        size left = end - src;
        size avail = b->cap - b->len;
        size amt = avail < left ? avail : left;

        for (size i = 0; i < amt; i++)
        {
            b->buf[b->len + i] = src[i];
        }
        b->len += amt;
        src += amt;

        if (amt < left)
        {
            flush(b);
        }
    }
}

#define append_str(b, s) append(b, (u8 *) s, sizeof(s) - 1)
#define append_s8(b, s)  append(b, s.data, s.len)

void append_long(struct buf *b, long x)
{
    u8 tmp[64];
    u8 *end = tmp + sizeof(tmp);
    u8 *beg = end;
    long t = x > 0 ? -x : x;
    do
    {
        *--beg = (u8) ('0' - t % 10);
    } while (t /= 10);
    if (x < 0)
    {
        *--beg = '-';
    }
    append(b, beg, end - beg);
}

struct code
{
    u8  *base;
    size len;
};

struct binary
{
    struct code exec;
    size pad1;
    struct code thunk;
    struct code chunk;
    size pad2;
};

#define MARKER 0xDB

static bool is_exec(struct arena scratch, struct s8 fname)
{
    Elf32_Ehdr ehdr;
    bool is_exec = false;

    int fd = s8open(scratch, fname, O_RDONLY);
    if (fd < 0)
    {
        append_str(stdout, "error: opening ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }
    
    if (SYSCALL3(SYS_read, fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
    {
        append_str(stdout, "error: reading");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    if (ehdr.e_ident[0] != ELFMAG0
        || ehdr.e_ident[1] != ELFMAG1
        || ehdr.e_ident[2] != ELFMAG2
        || ehdr.e_ident[3] != ELFMAG3
        || ehdr.e_type != ET_EXEC
        || ehdr.e_machine != EM_ARM
        || ehdr.e_version != EV_CURRENT)
    {
        append_str(stdout, "error: ");
        append_s8(stdout, fname);
        append_str(stdout, " is not an executable\n");
        goto cleanup;
    }

    if ((ehdr.e_ident[EI_NIDENT - 1] & MARKER) == MARKER)
    {
        append_str(stdout, "error: ");
        append_s8(stdout, fname);
        append_str(stdout, " has alredy been injected\n");
        goto cleanup;
    }
    is_exec = true;

cleanup:
    SYSCALL1(SYS_close, fd);
    return is_exec;
}

static bool load_exec(struct arena scratch, struct s8 fname, struct code *exec)
{
    bool loaded = false;
    
    int fd = s8open(scratch, fname, O_RDWR);
    if (fd < 0)
    {
        append_str(stdout, "error: opening ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    struct stat64 sbuf;    
    if (SYSCALL2(SYS_fstat64, fd, &sbuf) < 0)
    {
        append_str(stdout, "error: statting ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    u8 *base;
    base = (u8 *) SYSCALL6(SYS_mmap2, NULL, sbuf.st_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE, fd, 0);
    if (base == MAP_FAILED)
    {
        append_str(stdout, "error: mapping ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }
    
    exec->len = (size) sbuf.st_size;
    exec->base = base;
    loaded = true;

cleanup:    
    SYSCALL1(SYS_close, fd);
    return loaded;
}

static void load_thunk(struct code *thunk)
{
    static u8 _thunk[] = 
    {
        0xff, 0x4f, 0x2d, 0xe9, 0x34, 0x00, 0x9d, 0xe5, 
        0x38, 0x10, 0x8d, 0xe2, 0x01, 0x60, 0xa0, 0xe1, 
        0x00, 0x80, 0xa0, 0xe1, 0x05, 0x70, 0xa0, 0xe3, 
        0x00, 0x00, 0x91, 0xe5, 0x00, 0x10, 0xa0, 0xe3, 
        0x00, 0x00, 0x00, 0xef, 0x01, 0x00, 0x70, 0xe3, 
        0x00, 0x40, 0xa0, 0xe1, 0x14, 0x00, 0x00, 0x0a, 
        0x6c, 0xa0, 0x9f, 0xe5, 0x01, 0x00, 0xa0, 0xe1, 
        0x0a, 0xa0, 0x8f, 0xe0, 0xc0, 0x70, 0xa0, 0xe3, 
        0x00, 0x10, 0x9a, 0xe5, 0x05, 0x20, 0xa0, 0xe3, 
        0x04, 0x50, 0x9a, 0xe5, 0x02, 0x30, 0xa0, 0xe3, 
        0x00, 0x00, 0x00, 0xef, 0x01, 0x00, 0x70, 0xe3, 
        0x00, 0x50, 0xa0, 0xe1, 0x08, 0x00, 0x00, 0x0a, 
        0x08, 0x30, 0x9a, 0xe5, 0x06, 0x10, 0xa0, 0xe1, 
        0x08, 0x00, 0xa0, 0xe1, 0x03, 0x30, 0x85, 0xe0, 
        0x33, 0xff, 0x2f, 0xe1, 0x5b, 0x70, 0xa0, 0xe3, 
        0x00, 0x10, 0x9a, 0xe5, 0x05, 0x00, 0xa0, 0xe1, 
        0x00, 0x00, 0x00, 0xef, 0x06, 0x70, 0xa0, 0xe3, 
        0x04, 0x00, 0xa0, 0xe1, 0x00, 0x00, 0x00, 0xef, 
        0x10, 0xc0, 0x9f, 0xe5, 0x0c, 0xc0, 0x8f, 0xe0, 
        0x0c, 0xc0, 0x9c, 0xe5, 0xff, 0x4f, 0xbd, 0xe8, 
        0x1c, 0xff, 0x2f, 0xe1, 0x6c, 0x00, 0x00, 0x00, 
        0x10, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 
        0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 
        0x04, 0x00, 0x00, 0x00
    };
    
    thunk->len = sizeof(_thunk);
    thunk->base = _thunk;
}

static bool load_chunk(struct arena scratch, struct s8 fname, size chunk_entry_off, struct code *chunk)
{
    bool loaded = false;

    int fd = s8open(scratch, fname, O_RDONLY);
    if (fd < 0)
    {
        append_str(stdout, "error: opening ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    struct stat64 sbuf;    
    if (SYSCALL2(SYS_fstat64, fd, &sbuf) < 0)
    {
        append_str(stdout, "error: statting ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }
        
    if (chunk_entry_off > sbuf.st_size - 4)
    {
        append_str(stdout, "error: invalid entry offset\n");
        goto cleanup;
    }

    u8 *base;
    base = (u8 *) SYSCALL6(SYS_mmap2, NULL, sbuf.st_size, PROT_READ,
                           MAP_PRIVATE, fd, 0);
    if (base == MAP_FAILED)
    {
        append_str(stdout, "error: mapping ");
        append_s8(stdout, fname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    chunk->len = (size) sbuf.st_size;
    chunk->base = base;
    loaded = true;

cleanup:
    SYSCALL1(SYS_close, fd);
    return loaded;
}

static bool load(struct arena scratch, struct s8 cname, struct s8 ename, size chunk_entry_off,
                 struct binary *bin)
{
    load_thunk(&bin->thunk);
    return load_exec(scratch, ename, &bin->exec)
        && load_chunk(scratch, cname, chunk_entry_off, &bin->chunk);
}

static Elf32_Phdr *get_text_phdr(Elf32_Ehdr *ehdr, Elf32_Phdr *phdrs)
{
    for (i32 i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_type == PT_LOAD
            && (phdrs[i].p_flags & PF_R)
            && (phdrs[i].p_flags & PF_X))
        {
            // Assumption is that the first loadable segment that's
            // read executable in the file is the text segment.
            return phdrs + i;
        }
    }
    return NULL;
}

// Offset of entry point in thunk
static size thunk_entry_off = 0;
// Offsets inside of thunk of data that will be patched in
// at injection time (see thunk.c for how each one is used).
static size off_chunk_off = 176;
static size off_chunk_len = 172;
static size off_chunk_entry_off = 180;
static size off_host_entry = 184;

static bool inject(struct binary *bin, size chunk_entry_off, size *off)
{
    assert(bin->thunk.len < PGSIZE && !(bin->thunk.len& 0x3));
    
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) bin->exec.base;
    Elf32_Phdr *phdrs = (Elf32_Phdr *) (bin->exec.base + ehdr->e_phoff);
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (bin->exec.base + ehdr->e_shoff);
    Elf32_Phdr *text_phdr;
    if (!(text_phdr = get_text_phdr(ehdr, phdrs)))
    {
        append_str(stdout, "error: can't find text segment\n");
        return false;
    }

    size ins_off = (size) (text_phdr->p_offset + text_phdr->p_filesz);
    if (ins_off < 0)
    {
        append_str(stdout, "error: invalid insert position\n");
        return false;
    }

    // The insertion position must be a multiple of 4, pad before the thunk
    // if necessary.
    bin->pad1 = -ins_off & 3;
    size thunk_len = bin->pad1 + bin->thunk.len;
    thunk_entry_off += bin->pad1;

    // The thunk must be small enough to fit in the padding at the end of the
    // text section,  If if's larger, it might spill over into the next
    // segment in memory if it happens to be mapped adjacent to the end
    // of the text segment.
    if (thunk_len > (-ins_off & (PGSIZE - 1)))
    {
        append_str(stdout, "error: not enough padding\n");
        return false;
    }
    
    // File offsets and virtual address of each segment must be modular         
    // congruent to the page size.  The code is inserted directly after
    // the thunk so there must be adequate padding to ensure the total
    // number of bytes inserted in a multiple of page size.
    bin->pad2 = -(thunk_len + bin->chunk.len) & (PGSIZE - 1);
    size chunk_len = bin->pad2 + bin->chunk.len;

    // Update all segment and section file offsets that occur after the
    // insertion position by the total number of bytes inserted.
    size total_len = thunk_len + chunk_len;
    assert(total_len % PGSIZE == 0);
    
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
    
    // Since the thunk is inserted at the end of the text segment, the size
    // of the last section header in the text segment must be increased by
    // the length of the thunk.
    for (i32 i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].sh_offset + shdrs[i].sh_size == (usize) ins_off)
        {
            // The last section must not be stripped out when loaded.
            if (shdrs[i].sh_type != SHT_PROGBITS)
            {
                append_str(stdout, "error: last section stripped\n");
                return false;
            }
            shdrs[i].sh_size += (usize) thunk_len;
            break;
        }
    }
    
    // Patch the thunk so it can find and execute the chunk injected into the file.
    size chunk_off = ins_off + thunk_len;
    size tchunk_off = chunk_off & ~(PGSIZE - 1);
    size tchunk_len = chunk_off - tchunk_off + chunk_len;
    size tchunk_entry_off = chunk_off - tchunk_off + chunk_entry_off;
    // Thunk uses mmap2 which uses page multiples of offsets instead of number of
    // bytes.
    tchunk_off /= PGSIZE;
    memcpy(bin->thunk.base + off_chunk_off, &tchunk_off, sizeof(tchunk_off));
    memcpy(bin->thunk.base + off_chunk_len, &tchunk_len, sizeof(tchunk_len));
    memcpy(bin->thunk.base + off_chunk_entry_off, &tchunk_entry_off, sizeof(tchunk_entry_off));
    // Patch the thunk so that it will call the original entry point when it
    // finishes.
    memcpy(bin->thunk.base + off_host_entry, &ehdr->e_entry, sizeof(ehdr->e_entry));
    
    // Adjust the target's entry point so it will call the thunk when the
    // executable is started,
    ehdr->e_entry = text_phdr->p_vaddr + text_phdr->p_filesz
        + (usize) thunk_entry_off;
    
    // Increase the size of the text segment in the file and in memory by thunk
    // size.  
    text_phdr->p_filesz += (usize) thunk_len;
    text_phdr->p_memsz += (usize) thunk_len;
    // The section headers should always be after the insertion.
    if (ehdr->e_shoff > (usize) ins_off)
    {
        ehdr->e_shoff += (usize) total_len;
    }

    // Mark as injected so it can't be injected a second time.  A portion of
    // the magic number and other info is used, use it for this purpose.
    ehdr->e_ident[EI_NIDENT - 1] = MARKER;
    
    *off = ins_off;
    return true;
}

bool output(struct arena scratch, struct s8 fname, struct binary *bin,
            size ins_off)
{
    bool outputted = false;
    
    struct s8 tname = s8cat(&scratch, fname, s8(".injected"));
    int fd = s8open(scratch, tname, O_WRONLY | O_TRUNC | O_CREAT);
    if (fd < 0)
    {
        append_str(stdout, "error: opening ");
        append_s8(stdout, tname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    struct stat64 sbuf;
    if (s8stat64(scratch, fname, &sbuf) < 0)
    {
        append_str(stdout, "error: statting ");
        append_s8(stdout, tname);
        append_str(stdout, "\n");
        goto cleanup;
    }
    
    u8 *pad1 = new(&scratch, u8, bin->pad1);
    u8 *pad2 = new(&scratch, u8, bin->pad2);
    size remain_len = bin->exec.len - ins_off;
    if (SYSCALL3(SYS_write, fd, bin->exec.base, (usize) ins_off) != ins_off
        || SYSCALL3(SYS_write, fd, pad1, (usize) bin->pad1) != bin->pad1
        || SYSCALL3(SYS_write, fd, bin->thunk.base, (usize) bin->thunk.len) != bin->thunk.len
        || SYSCALL3(SYS_write, fd, bin->chunk.base, (usize) bin->chunk.len) != bin->chunk.len
        || SYSCALL3(SYS_write, fd, pad2, (usize) bin->pad2) != bin->pad2
        || SYSCALL3(SYS_write, fd, bin->exec.base + ins_off, (usize) remain_len) != remain_len)
    {
        append_str(stdout, "error: writing ");
        append_s8(stdout, tname);
        append_str(stdout, "\n");
        goto cleanup;
    }

    if (s8chmod(scratch, tname, sbuf.st_mode) < 0
        || s8chown(scratch, tname, sbuf.st_uid, sbuf.st_gid) < 0)
    {
        append_str(stdout, "error: udpating ");
        append_s8(stdout, tname);
        append_str(stdout, "\n");
        goto cleanup;
    }
    outputted = true;

cleanup:
    if (!outputted)
    {
        s8unlink(scratch, tname);
    }
    SYSCALL1(SYS_close, fd);
    return outputted;
}

int start(int argc, char **argv)
{
    volatile int success = -1;
    jmp_buf ctx;
    struct arena perm = { 0 };
    struct arena scratch = { 0 };
    if (!new_arena(&perm, &ctx, 1 << 12)
        || !new_arena(&scratch, &ctx, 1 << 12)
        || setjmp(ctx))
    {
        append_str(stdout, "error: out of memory\n");
        goto cleanup;
    }
    
    if (argc != 4)
    {
        append_str(stdout, "usage: elf_injector <exec> <chunk> <entry offset>\n");
        goto cleanup;
    }

    struct s8 ename = s8cstr(argv[1]);
    struct s8 cname = s8cstr(argv[2]);    
    if (!is_exec(scratch, ename))
    {
        goto cleanup;
    }
    
    size chunk_entry_off = atoi(argv[3]);
    if (chunk_entry_off < 0)
    {
        append_str(stdout, "error: invalid entry offset\n");
        goto cleanup;
    }

    struct binary bin = { 0 };
    size ins_off;
    if (!load(scratch, cname, ename, chunk_entry_off, &bin)
        || !inject(&bin, chunk_entry_off, &ins_off)
        || !output(scratch, ename, &bin, ins_off))
    {
        goto cleanup;
    }
    success = 0;

cleanup:
    flush(stdout);
    return success;
}

__attribute__((naked, noreturn))
void _start(void)
{
    __asm volatile (
        "ldr     r0, [sp]\n"
        "add     r1, sp, #4\n"
        "bl      start\n"
        "mov     r7, #1\n"
        "swi     #0\n"
        );
}
