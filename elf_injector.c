// LC_ALL=C LANG=C gcc -Werror -std=c99 -Wall -Wextra -Wno-error=unused-parameter -Wno-error=unused-function -Wno-error=unused-variable -Wconversion -Wno-error=sign-conversion -fsanitize=address,undefined -fno-diagnostics-color -g3 -o elf_injector elf_injector.c
// LD_PRELOAD=/lib/arm-linux-gnueabihf/libasan.so.6 ./elf_injector
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

#define sizeof(x)        (ptrdiff_t) sizeof(x)
#define alignof(x)       (ptrdiff_t) _Alignof(x)
#define countof(a)       (sizeof(a) / sizeof(*(a)))
#define lengthof(s)      (countof(s) - 1)
#define s8(s)            (struct s8){(u8 *)s, countof(s)-1}

#define APPEND_STR(b, s) append(b, s.data, s.len)
#define MEMBUF(buf, cap) { buf, cap, 0, 0 }

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

#define PAGE_SZ 4096

static struct s8 s8cstr(char *s)
{
    struct s8 r = {0};
    r.data = (u8 *) s;
    r.len = (size) strlen(s);
    return r;
}

static void append(struct buf *buf, u8 *src, size len)
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
                          size target_sz, size insert_pos,
                          u8 *code_buf)
{
    u8 buf[4096];
    struct buf output_fname = MEMBUF(buf, sizeof(buf));
    APPEND_STR(&output_fname, target_fname);
    APPEND_STR(&output_fname, s8(".injected"));

    int output_fd;
    if ((output_fd = open((char *) output_fname.data,
                          O_WRONLY | O_TRUNC | O_CREAT, 0755)) < 0)
    {
        perror("open");
        return false;
    }
    if (write(output_fd, target_buf, (usize) insert_pos) != insert_pos
        || write(output_fd, code_buf, PAGE_SZ) != PAGE_SZ
        || write(output_fd, target_buf + insert_pos,
                 (usize) (target_sz - insert_pos)) != target_sz - insert_pos)
    {
        perror("write");
        return false;
    }
    close(output_fd);
    return true;
}

static bool inject_code(struct s8 target_fname, u8 *target_buf,
                        size target_sz, u8 *code_buf, size code_sz,
                        size entry_offset, size host_entry_offset)
{
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
    // Is there enough padding available for code to be inserted?
    size insert_pos = (size) (text_phdr->p_offset + text_phdr->p_filesz);
    if (insert_pos < 0)
    {
        fprintf(stderr, "error: invalid insert position\n");
        return false;
    }
    size padding = -(usize) insert_pos & (PAGE_SZ - 1);
    if (code_sz > padding)
    {
        fprintf(stderr, "error: not enough padding\n");
        return false;
    }
    // File offsets and virtual address of each segment must be modular         
    // congruent to the page size.  For this reason, a page of data must be
    // inserted into the file even though code size is less than a page.
    // Adjust all segments and sections file offsets that occur after the
    // text segment by a page to account for this.
    for (i32 i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_offset > text_phdr->p_offset + text_phdr->p_filesz)
        {
            phdrs[i].p_offset += PAGE_SZ;
        }
    }
    for (i32 i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].sh_offset > text_phdr->p_offset + text_phdr->p_filesz)
        {
            shdrs[i].sh_offset += PAGE_SZ;
        }
    }
    // Since code is inserted at the end of the text segment, the size of the
    // last section header in the text segment must be increased by code size.
    for (i32 i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].sh_offset + shdrs[i].sh_size == (usize) insert_pos)
        {
            // The last section must not be stripped out when loaded.
            if (shdrs[i].sh_type != SHT_PROGBITS)
            {
                fprintf(stderr, "error: last section stripped\n");
                return false;
            }
            shdrs[i].sh_size += (usize) code_sz;
            break;
        }
    }
    // Patch the inserted code so that it will call the original entry
    // point when it finishes.
    memcpy(code_buf + host_entry_offset, &ehdr->e_entry, sizeof(ehdr->e_entry));
    // Adjust the target's entry point so it will call the inserted code
    // when the executable is started. 
    ehdr->e_entry = text_phdr->p_vaddr + text_phdr->p_filesz
        + (usize) entry_offset;
    // Increase the size of the text segment in the file and in memory by the
    // code size.  Even though a page of data is going to be inserted
    // into the file, the sizes must only be adjusted by code size to avoid
    // possibly spilling over to the next segment in memory if it happens to
    // be mapped adjacent to the end of the text segment.
    text_phdr->p_filesz += (usize) code_sz;
    text_phdr->p_memsz += (usize) code_sz;
    // The section headers should always be after the insertion.
    if (ehdr->e_shoff > (usize) insert_pos)
    {
        ehdr->e_shoff += PAGE_SZ;
    }
    if (!output_target(target_fname, target_buf, target_sz, insert_pos,
                       code_buf))
    {
        return false;
    }
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        printf("usage: elf_injector <target> <code> <entry offset>"
               " <host entry offset>\n");
        return EXIT_FAILURE;
    }

    struct s8 target_fname = s8cstr(argv[1]);
    struct s8 code_fname = s8cstr(argv[2]);
    int target_fd;
    int code_fd;
    if ((target_fd = open((char *) target_fname.data, O_RDONLY)) < 0
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

    size entry_offset = atoi(argv[3]);
    if (entry_offset < 0 || entry_offset > target_stat.st_size - 1)
    {
        fprintf(stderr, "error: invalid entry offset\n");
        return EXIT_FAILURE;
    }
    
    size host_entry_offset = atoi(argv[4]);
    if (host_entry_offset < 0 || host_entry_offset > target_stat.st_size - 1)
    {
        fprintf(stderr, "error: invalid host entry offset\n");
        return EXIT_FAILURE;
    }

    if (code_stat.st_size > PAGE_SZ)
    {
        fprintf(stderr, "error: code size can't exceed %d bytes\n",
            PAGE_SZ);
        return EXIT_FAILURE;
    }

    u8 code_buf[PAGE_SZ];
    memset(code_buf, 0xFF, PAGE_SZ);
    if (read(code_fd, code_buf, (usize) code_stat.st_size) != code_stat.st_size)
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

    if (!inject_code(target_fname, target_buf, target_stat.st_size, code_buf,
                     code_stat.st_size, entry_offset, host_entry_offset))
    {
        return EXIT_FAILURE;
    }
        
    munmap(target_buf, (usize) target_stat.st_size);
    close(target_fd);
    return EXIT_SUCCESS;
}
