// gcc -Werror -std=c99 -Wall -Wextra -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -Wconversion -Wno-error=sign-conversion -fsanitize=address,undefined -fno-diagnostics-color -g3 -fno-omit-frame-pointer -o elf_xinjector elf_xinjector.c
// LD_PRELOAD=/lib/arm-linux-gnueabihf/libasan.so.6 ./elf_xinjector
// To debug with gdb: set environment ASAN_OPTIONS=abort_on_error=1:detect_leak=0
//                    set environment LD_PRELOAD=/lib/arm-linux-gnueabihf/libasan.so.6 
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

#define BITMASK(SHIFT, CNT) (((1ul << (CNT)) - 1) << (SHIFT))
#define PGSHIFT 0                          
#define PGBITS  12                         
#define PGSIZE  (1 << PGBITS)              
#define PGMASK  BITMASK(PGSHIFT, PGBITS)   

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

static size sz_round_up (size sz)
{
    return (size) (((usize) addr + PGSIZE - 1) & ~PGMASK);
}

static void *addr_round_up (void *addr)
{
    return (void *) (((uptr) addr + PGSIZE - 1) & ~PGMASK);
}

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
    size padding = -(iptr) arena->beg & (align - 1);
    size available = arena->end - arena->beg - padding;
    if (available < 0 || count >  available / sz)
    {
        return NULL;
    }
    u8 *p = arena->beg + padding;
    arena->beg += padding + count * sz;
    if (arena->beg >= *arena->commit)
    {
        if (mprotect(*arena->commit, (size_t) (arena->beg - *arena->commit + 1),
                     PROT_READ | PROT_WRITE) == -1)
        {
            return NULL;
        }
        *arena->commit = addr_round_up(arena->beg);
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
                          size target_sz, size insert_offset,
                          u8 *code_buf, size thunk_code_len)
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
        perror("open");
        return false;
    }
    
    size remain_len = target_sz - insert_offset;
    if (write(output_fd, target_buf, (usize) insert_offset) != insert_offset
        || write(output_fd, code_buf, thunk_code_len) != thunk_code_len
        || write(output_fd, target_buf + insert_offset, remain_len) != remain_len)
    {
        perror("write");
        return false;
    }
    close(output_fd);
    return true;
}

static bool inject_code(struct s8 target_fname, u8 *target_buf,
                        size target_len, u8 *thunk_code_buf, size code_len,
                        size thunk_code_len, size code_entry_offset)

{
    assert(thunk_code_len % PGSIZE == 0);
    Elf32_Ehdr *ehdr;
    if (!(ehdr = get_ehdr(target_buf)))
    {
        return false;
    }
    
    Elf32_Phdr *phdrs = (Elf32_Phdr *) (target_buf + ehdr->e_phoff);
    Elf32_Shdr *shdrs = (Elf32_Shdr *) (target_buf + ehdr->e_shoff);
    Elf32_Phdr *text_phdr;
    if (!(text_phdr = get_text_phdr(ehdr, phdrs)))
    {
        return false;
    }
    // Is there enough padding available for the thunk to be inserted?
    size insert_offset = (size) (text_phdr->p_offset + text_phdr->p_filesz);
    if (insert_offset < 0)
    {
        fprintf(stderr, "error: invalid insert position\n");
        return false;
    }

    // The thunk must be small enough to fit in the padding at the end of the
    // text section.  If if's larger, it might spill over into the next
    // segment in memory if it happens to be mapped adjacent to the end
    // of the text segment.
    size padding = -(usize) insert_offset & (PGSIZE - 1);
    if (sizeof(thunk) > padding)
    {
        fprintf(stderr, "error: not enough padding\n");
        return false;
    }
    // File offsets and virtual address of each segment must be modular         
    // congruent to the page size.  For this reason, the thunk and the code,
    // code_len, have been rounded up to the next page.  Adjust all segment
    // and section file offsets that occur after the text segment by the
    // requied length.
    for (i32 i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_offset > insert_offset)
        {
            phdrs[i].p_offset += thunk_code_len;
        }
    }
    for (i32 i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].sh_offset > insert_offset)
        {
            shdrs[i].sh_offset += thunk_code_len;
        }
    }
    // Since the thunk is inserted at the end of the text segment, the size
    // of the last section header in the text segment must be increased by
    // thunk size.
    for (i32 i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].sh_offset + shdrs[i].sh_size == (usize) insert_offset)
        {
            // The last section must not be stripped out when loaded.
            if (shdrs[i].sh_type != SHT_PROGBITS)
            {
                fprintf(stderr, "error: last section stripped\n");
                return false;
            }
            shdrs[i].sh_size += sizeof(thunk);
            break;
        }
    }
    // Patch the thunk so it can find and execute the code within the file.
    size code_offset = insert_offset + sizeof(thunk);
    memcpy(code_buf + thunk_offset_code_offset, &code_offset,
           sizeof(code_offset));
    memcpy(code_buf + thunk_offset_code_len, &code_len, sizeof(code_len));
    memcpy(code_buf + thunk_offset_entry_offsett, &code_entry_offset,
           sizeof(code_entry_offset));
    // Patch the thunk so that it will call the original entry point when it
    // finishes.
    memcpy(code_buf + thunk_host_entry_offset, &ehdr->e_entry,
           sizeof(ehdr->e_entry));
    // Adjust the target's entry point so it will call the thunk when the
    // executable is started,
    ehdr->e_entry = text_phdr->p_vaddr + text_phdr->p_filesz
        + (usize) thunk_entry_offset;
    // Increase the size of the text segment in the file and in memory by thunk size.  
    text_phdr->p_filesz += sizeof(thunk);
    text_phdr->p_memsz += sizeof(thunk);
    // The section headers should always be after the insertion.
    if (ehdr->e_shoff > (usize) insert_offset)
    {
        ehdr->e_shoff += thunk_code_len;
    }
    if (!output_target(target_fname, target_buf, target_sz, insert_offset,
                       thunk_code_buf, thunk_code_len))
    {
        return false;
    }
    return true;
}

// TODO thunk must be less than a page
static u8 thunk[] =
{
    0x00, 0x00, 0x00
};

static size thunk_entry_offset = -1;   
static size thunk_host_entry_offset = -2;

// Offsets in thunk for locations that need to be patched at injection so it's
// able to load and execute the injected code.
static size thunk_offset_code_offset = -3;
static size thunk_offset_code_len = -4;
static size thunk_offset_entry_offset = -5

int main(int argc, char **argv)
{
    assert(sizeof thunk < PGSIZE);
    if (argc != 5)
    {
        printf("usage: elf_xinjector <target> <code> <entry offset>\n");
        return EXIT_FAILURE;
    }

    struct s8 target_fname = s8cstr(argv[1]);
    struct s8 code_fname = s8cstr(argv[2]);
    int target_fd;
    int code_fd;
    if ((target_fd = open((char *) target_fname.data, O_RDWR)) < 0
        || (code_fd = open((char *) code_fname.data, O_RDONLY)) < 0)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat target_stat;
    struct stat code_stat;
    if (fstat(target_fd, &target_stat) < 0
        || fstat(code_fd, &code_stat) < 0)
    {
        perror("fstat");
        return EXIT_FAILURE;
    }

    size code_entry_offset = atoi(argv[3]);
    if (code_entry_offset < 0 || code_entry_offset > code_stat.st_size - 1)
    {
        fprintf(stderr, "error: invalid entry offset\n");
        return EXIT_FAILURE;
    }

    sz thunk_code_len = sz_round_up(sizeof(thunk) + code_stat.st_size);
    struct arena arena;
    u8 *thunk_code_buf;
    if (!new_arena(&arena, thunk_code_len * 2)
        || !(thunk_code_buf = new(&arena, u8, thunk_code_len)))
    {
        fprintf(stderr, "error: out of memory\n");
        return EXIT_FAILURE;
    }
    // The code is adjacent to the thunk.
    memcpy(thunk_code_buf, thunk, sizeof(thunk));
    if (read(code_fd, thunk_code_buf + sizeof(thunk), (usize) code_stat.st_size)
        != code_stat.st_size)
    {
        perror("read");
        return EXIT_FAILURE;
    }
    close(code_fd);
                
    void *target_buf;
    if ((target_buf = mmap(NULL, (usize) target_stat.st_size,
                           PROT_READ | PROT_WRITE, MAP_PRIVATE,
                           target_fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        return EXIT_FAILURE;
    }

    if (!inject_code(target_fname, target_buf, target_stat.st_size,
                     thunk_code_buf, code_stat.st_size, thunk_code_len,
                     code_entry_offset))
    {
        return EXIT_FAILURE;
    }
        
    munmap(target_buf, (usize) target_stat.st_size);
    close(target_fd);
    return EXIT_SUCCESS;
}
