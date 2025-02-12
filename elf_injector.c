#define _DEFAULT_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>

typedef uint8_t   u8;
typedef int32_t   i32;
typedef uint32_t  u32;
typedef int64_t   i64;
typedef uint64_t  u64;
typedef float     f32;
typedef double    f64;
typedef uintptr_t uptr;
typedef intptr_t  iptr;
typedef char      byte;
typedef ptrdiff_t size;
typedef size_t    usize;

#define sizeof(x)    (size) sizeof(x)
#define alignof(x)   (size) _Alignof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)
#define s8(s)            (struct s8){(u8 *)s, lengthof(s)}
#define new(a, t, n) (t *) alloc(a, sizeof(t), _Alignof(t), n)

#define APPEND_STR(b, s) append(b, (const u8 *) s, sizeof(s) - 1)
#define APPEND_S8(b, s)  append(b, s.data, s.len)
#define APPEND_NUL(b)    append(b, (const u8 *) "", 1)
#define MEMBUF(buf, cap) { buf, cap, 0, 0 }

#define PGSIZE  (1 << 12)

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
static size thunk_len = sizeof(thunk);
static size thunk_entry_off = 0;
// Offsets in thunk for locations that need to be patched at injection so it's
// able to load and execute the injected code.
static size thunk_off_code_off = 176;
static size thunk_off_code_len = 172;
static size thunk_off_code_entry_off = 180;
static size thunk_off_host_entry = 184;

struct s8
{
    u8  *data;
    size len;
};

struct buf
{
    u8 *data;
    size cap;
    size len;
    bool error;
};

struct arena
{
    u8 *beg;
    u8 *end;
    u8 **commit;
};

static struct s8 s8cstr(const char *s)
{
    struct s8 r = {0};
    r.data = (u8 *) s;
    r.len = (size) strlen(s);
    return r;
}

static void append(struct buf *buf, const u8 *src, size len)
{
    size avail = buf->cap - buf->len;
    size amount = avail < len ? avail : len;
    for (size i = 0; i < amount; i++)
    {
        buf->data[buf->len + i] = src[i];
    }
    buf->len += amount;
    buf->error |= amount < len;
}

static bool new_arena(struct arena *arena, size sz)
{
    if (sz <= 0)
    {
        return false;
    }
    sz += sizeof(arena->commit);
    arena->beg = mmap(NULL, (usize) sz, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (arena->beg == MAP_FAILED)
    {
        return false;
    }
    if (mprotect(arena->beg, PGSIZE, PROT_READ | PROT_WRITE) == -1)
    {
        return false;
    }
    arena->end = arena->beg + sz;
    arena->commit = (u8 **) arena->beg;
    *arena->commit = arena->beg + PGSIZE;
    arena->beg += sizeof(arena->commit);
    return true;
}

static void *alloc(struct arena *arena, size sz, size align, size count)
{
    size pad = -(iptr) arena->beg & (align - 1);
    size avail = arena->end - arena->beg - pad;
    if (avail < 0 || count >  avail / sz)
    {
        return NULL;
    }
    u8 *p = arena->beg + pad;
    arena->beg += pad + count * sz;
    if (arena->beg >= *arena->commit)
    {
        if (mprotect(*arena->commit, (size_t) (arena->beg - *arena->commit + 1),
                     PROT_READ | PROT_WRITE) == -1)
        {
            return NULL;
        }
        *arena->commit = (u8 *) ((iptr) (arena->beg + (PGSIZE - 1)) & ~(PGSIZE - 1));
    }
    return memset(p, 0, (size_t) (count * sz));
}

static Elf32_Ehdr *get_ehdr(void *buf)
{
    Elf32_Ehdr *ehdr = buf;
    if (ehdr->e_ident[0] != ELFMAG0
        || ehdr->e_ident[1] != ELFMAG1
        || ehdr->e_ident[2] != ELFMAG2
        || ehdr->e_ident[3] != ELFMAG3)
    {
        fprintf(stderr, "error: not ELF file\n");
        return NULL;
    }
    if (ehdr->e_type != ET_EXEC)
    {
        fprintf(stderr, "error: not executable\n");
        return NULL;
    }
    if (ehdr->e_machine != EM_ARM)
    {
        fprintf(stderr, "error: invalid architecture\n");
        return NULL;
    }
    if (ehdr->e_version != EV_CURRENT)
    {
        fprintf(stderr, "error: invalid version\n");
        return NULL;
    }
    return buf;
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
    fprintf(stderr, "error: can't find text segment\n");
    return NULL;
}

static bool output_target(struct s8 target_fname, u8 *target_buf,
                          size target_len, size insert_off,
                          u8 *thunk_buf, size thunk_len,
                          u8 *code_buf, size code_len)
{
    u8 buf[1 << 12];
    struct buf output_fname = MEMBUF(buf, sizeof(buf));
    APPEND_S8(&output_fname, target_fname);
    APPEND_STR(&output_fname, ".injected");
    APPEND_NUL(&output_fname);

    int output_fd;
    if ((output_fd = open((char *) output_fname.data,
                          O_WRONLY | O_TRUNC | O_CREAT, 0755)) < 0)
    {
        fprintf(stderr, "error: opening %s: %s\n", (char *) output_fname.data,
                strerror(errno));
        return false;
    }
    
    size remain_len = target_len - insert_off;
    if (write(output_fd, target_buf, (usize) insert_off) != insert_off
        || write(output_fd, thunk_buf, (usize) thunk_len) != thunk_len
        || write(output_fd, code_buf, (usize) code_len) != code_len
        || write(output_fd, target_buf + insert_off, (usize) remain_len) != remain_len)
    {
        fprintf(stderr, "error: writing to %s: %s\n", (char *) output_fname.data,
                strerror(errno));
        return false;
    }
    close(output_fd);
    return true;
}

static bool inject_code(struct arena *arena, struct s8 target_fname, u8 *target,
                        size target_len, u8 *code, size code_len, size code_entry_off)
{
    Elf32_Ehdr *ehdr;
    if (!(ehdr = get_ehdr(target)))
    {
        return false;
    }
    
    Elf32_Phdr *phdrs = (Elf32_Phdr *) (target + ehdr->e_phoff);
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (target + ehdr->e_shoff);
    Elf32_Phdr *text_phdr;
    if (!(text_phdr = get_text_phdr(ehdr, phdrs)))
    {
        return false;
    }

    size insert_off = (size) (text_phdr->p_offset + text_phdr->p_filesz);
    if (insert_off < 0)
    {
        fprintf(stderr, "error: invalid insert position\n");
        return false;
    }

    // The insertion position must be a multiple of 2, pad the thunk if
    // necessary.
    size pad = -insert_off & 1;
    u8 *thunk_buf;
    if (!(thunk_buf = new(arena, u8, thunk_len + pad)))
    {
        fprintf(stderr, "error: out of memory\n");
        return false;        
    }
    memcpy(thunk_buf + pad, thunk, (usize) thunk_len);
    thunk_len += pad;
    thunk_entry_off += pad;
    thunk_off_code_off += pad;
    thunk_off_code_len += pad;
    thunk_off_code_entry_off += pad;
    thunk_off_host_entry += pad;

    // The thunk must be small enough to fit in the padding at the end of the
    // text section.  If if's larger, it might spill over into the next
    // segment in memory if it happens to be mapped adjacent to the end
    // of the text segment.
    pad = -insert_off & (PGSIZE - 1);
    if (thunk_len > pad)
    {
        fprintf(stderr, "error: not enough padding\n");
        return false;
    }
    
    // File offsets and virtual address of each segment must be modular         
    // congruent to the page size.  The code is inserted directly after
    // the thunk so there must be adequate padding to ensure the total
    // number of bytes inserted in a multiple of page size.
    pad = -(thunk_len + code_len) & (PGSIZE - 1);
    u8 *code_buf;
    if (!(code_buf = new(arena, u8, code_len + pad)))
    {
        fprintf(stderr, "error: out of memory\n");
        return false;        
    }
    memcpy(code_buf, code, (usize) code_len);
    code_len += pad;

    // Update all segment and section file offsets that occur after the
    // insertion position by the total number of bytes inserted.
    size total_len = thunk_len + code_len;
    assert(total_len % PGSIZE == 0);
    for (i32 i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_offset > (usize) insert_off)
        {
            phdrs[i].p_offset += (usize) total_len;
        }
    }
    for (i32 i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].sh_offset > (usize) insert_off)
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
    
    // Patch the thunk so it can find and execute the code within the file.
    size code_off = insert_off + thunk_len;
    size thunk_code_off = code_off & ~(PGSIZE - 1);
    size thunk_code_len = code_off - thunk_code_off + code_len;
    size thunk_code_entry_off = code_off - thunk_code_off + code_entry_off;
    // Thunk uses mmap2 which uses page multiples of offsets instead of number of
    // bytes.
    thunk_code_off /= PGSIZE;
    memcpy(thunk_buf + thunk_off_code_off, &thunk_code_off,
           sizeof(thunk_code_off));
    memcpy(thunk_buf + thunk_off_code_len, &thunk_code_len,
           sizeof(thunk_code_len));
    memcpy(thunk_buf + thunk_off_code_entry_off, &thunk_code_entry_off,
           sizeof(thunk_code_entry_off));
    // Patch the thunk so that it will call the original entry point when it
    // finishes.
    memcpy(thunk_buf + thunk_off_host_entry, &ehdr->e_entry, sizeof(ehdr->e_entry));
    // Adjust the target's entry point so it will call the thunk when the
    // executable is started,
    ehdr->e_entry = text_phdr->p_vaddr + text_phdr->p_filesz
        + (usize) thunk_entry_off;
    // Increase the size of the text segment in the file and in memory by thunk
    // size.  
    text_phdr->p_filesz += (usize) thunk_len;
    text_phdr->p_memsz += (usize) thunk_len;
    // The section headers should always be after the insertion.
    if (ehdr->e_shoff > (usize) insert_off)
    {
        ehdr->e_shoff += (usize) total_len;
    }
    if (!output_target(target_fname, target, target_len, insert_off,
                       thunk_buf, thunk_len, code_buf, code_len))
    {
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    assert(thunk_len < PGSIZE && !(thunk_len & 0x1));
    if (argc != 4)
    {
        fprintf(stderr, "usage: elf_injector <target> <code> <entry offset>\n");
        return EXIT_FAILURE;
    }

    struct s8 target_fname = s8cstr(argv[1]);
    int target_fd;
    struct stat target_stat;
    if ((target_fd = open((char *) target_fname.data, O_RDWR)) < 0
        || fstat(target_fd, &target_stat) < 0)
    {
        fprintf(stderr, "error: opening %s: %s\n", (char *) target_fname.data,
                strerror(errno));
        return EXIT_FAILURE;
                
    }
    
    struct s8 code_fname = s8cstr(argv[2]);
    int code_fd;
    struct stat code_stat;
    if ((code_fd = open((char *) code_fname.data, O_RDONLY)) < 0
        || fstat(code_fd, &code_stat) < 0)
    {
        fprintf(stderr, "error: opening %s: %s\n", (char *) code_fname.data,
                strerror(errno));
        return EXIT_FAILURE;
    }

    size code_entry_off = atoi(argv[3]);
    if (code_entry_off < 0 || code_entry_off > code_stat.st_size - 4)
    {
        fprintf(stderr, "error: invalid entry offset\n");
        return EXIT_FAILURE;
    }

    struct arena arena;
    if (!new_arena(&arena, 1 << 20))
    {
        fprintf(stderr, "error: out of memory\n");
        return EXIT_FAILURE;
    }

    u8 *code;
    if ((code = mmap(NULL, (usize) code_stat.st_size,
                     PROT_READ | PROT_WRITE, MAP_PRIVATE,
                     code_fd, 0)) == MAP_FAILED)
    {
        fprintf(stderr, "error: mapping %s: %s\n", (char *) code_fname.data,
                strerror(errno));
        return EXIT_FAILURE;
    }
    close(code_fd);
                
    u8 *target;
    if ((target = mmap(NULL, (usize) target_stat.st_size,
                       PROT_READ | PROT_WRITE, MAP_PRIVATE,
                       target_fd, 0)) == MAP_FAILED)
    {
        fprintf(stderr, "error: mapping %s: %s\n", (char *) target_fname.data,
                strerror(errno));
        return EXIT_FAILURE;
    }
    close(target_fd);

    if (!inject_code(&arena, target_fname, target, target_stat.st_size,
                     code, code_stat.st_size, code_entry_off))

    {
        return EXIT_FAILURE;
    }
    
    munmap(target, (usize) target_stat.st_size);
    munmap(target, (usize) code_stat.st_size);
    return EXIT_SUCCESS;
}
