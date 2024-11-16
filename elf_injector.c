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

#define sizeof(x)    (ptrdiff_t) sizeof(x)
#define alignof(x)   (ptrdiff_t) _Alignof(x)
#define countof(a)   (sizeof(a) / sizeof(*(a)))
#define lengthof(s)  (countof(s) - 1)

#define PAGE_SZ 4096

/*
  * Increase p_shoff by PAGE_SIZE in the ELF header
  * Patch the insertion code (parasite) to jump to the entry point
    (original)
  * Locate the text segment program header
    * Modify the entry point of the ELF header to point to the new
      code (p_vaddr + p_filesz)
    * Increase p_filesz by account for the new code (parasite)
    * Increase p_memsz to account for the new code (parasite)
  * For each phdr who's segment is after the insertion (text segment)
    * increase p_offset by  PAGE_SIZE
  * For the last shdr in the text segment
    * increase sh_len by the parasite length
  * For each shdr who's section resides after the insertion
    * Increase sh_offset by PAGE_SIZE
  * Physically insert the new code (parasite) and pad to PAGE_SIZE, into
    the file - text segment p_offset + p_filesz (original)


    
    find text segment header
    increase file and memory size by the size of entry

    increase offset for each phdr by a page if it's after insertion point
    increase offset for each shdr by a page if it's after insertion point
    increase the size of the last section in the text header by entry size
    adjust entry point
    insert code
    
    
    
    find entry point
    patch entry point 
    modify entry point to point to new entry
    
    find text segment
*/

static Elf32_Ehdr *get_ehdr(void *buf)
{
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
    if (ehdr.e_version != EV_CURRENT)
    {
        fprintf(stderr, "error: invalid version\n");
        return NULL;
    }
    return buf;
}

static Elf32_Phdr *get_text_phdr(Elf32_Ehdr *ehdr, Elf32_Phdr *phdrs)
{
    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_type == PT_LOAD && phdr[i].p_offset == 0)
        {
            // Assumption is that the first loadable segment starting
            // at offset 0 in the file is the text segment.
            return phdrs + i;
        }
    }
    fprintf(stderr, "error: can't find text segment\n");
    return NULL;

}

static bool insert_entry(int fd, u8 *target_buf, u8 *entry_buf, size entry_sz, size offset)
{
    Elf32_Ehdr *ehdr;
    if (!(ehdr = get_ehdr(target_buf)))
    {
        return false;
    }
    
    Elf32_Phdr *phdrs = (Elf32_Phdr *) target_buf + ehdr->e_phoff;
    Elf32_Shdr *shdrs = (Elf32_Shdr *) target_buf + ehdr->e_shoff;
    Elf32_Ehdr *text_phdr;
    if (!(text_phdr = get_text_phdr(phdrs)))
    {
        return false;
    }
    // Is there enough padding available for entry to be inserted?
    size padding = -(uptr) (text_phdr->p_vaddr + text_phdr->p_file_sz) & (PAGE_SZ - 1);
    if (entry_sz > padding)
    {
        fprintf(stderr, "error: not enough space to insert\n");
        return false;
    }
    // File offsets and virtual address of each segment must be modular
    // congruent to the page size.  For this reason, a page of data must be
    // inserted into the file even though entry size is less than a page.
    // Adjust all segments and sections file offsets that occur after the text
    // text segment by a page to account for this.
    for (int i = 0; i < ehdr->e_phnum; i++)
    {
        if (phdrs[i].p_offset > text_phdr->p_offset + text_phdr->file_sz)
        {
            phdrs[i].p_offset += PAGE_SZ;
        }
    }
    for (int i = 1; i < ehdr->e_shnum; i++)
    {
        if (shdrs[i].p_offset > text_phdr->p_offset + text_phdr->file_sz)
        {
            shdrs[i].p_offset += PAGE_SZ;
        }
    }
    // Increase the size of the text segment in the file and in memory by the
    // entry size.  Even though a page of data is going to be inserted
    // into the file, the sizes must only be adjusted by entry size to avoid
    // possibly spilling over to the next segment in memory.
    text_phdr->p_filesz += entry_sz;
    text_phdr->p_memsz += entry_sz;
    // Since entry is inserted at the end of the text segment, the size of the last
    // section header in the text segment must be increased by entry size.


    // increase the size of the last section in the text header by entry size
    // adjust entry point
    // insert code
    return true;
}

int main(int argc, char **argv)
{
    if (argc != 4)
    {
        printf("usage: insert_entry <target> <entry> <host entry offset>\n");
        return EXIT_FAILURE;
    }

    size offset = atoi(argv[3]);
    if (offset == 0)
    {
        fprintf(stderr, "error: invalid offset\n");
        return EXIT_FAILURE;
    }
    
    int target_fd;
    int entry_fd;
    if ((target_fd = open(argv[1], O_RDWR)) < 0
        || (entry_fd = open(argv[1], O_RDONLY)) < 0)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    struct stat target_stat;
    struct stat entry_stat;
    if (fstat(target_fd, &target_stat) < 0
        || fstat(entry_fd, &entry_stat) < 0)
    {
        perror("fstat");
        return EXIT_FAILURE;
    }

    if (entry_stat.st_size > PAGE_SZ)
    {
        fprintf(stderr, "error: entry size can't exceed %d bytes\n",
            PAGE_SZ);
        return EXIT_FAILURE;
    }

    u8 *entry_buf[PAGE_SIZE];
    if (read(entry_fd, entry_buf, entry_stat.st_size) != entry_stat.st_size)
    {
        fprintf(stderr, "error: failed to read entry\n");
        return EXIT_FAILURE;
    }
    close(entry_fd);
                
    void *target_buf;
    if ((target_buf = mmap(NULL, target_stat.st_size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE, target_fd, 0)) == MAP_FAILED)
    {
        perror("mmap");
        return EXIT_FAILURE;
    }

    if (!insert_entry(target_fd, target_buf, entry_buf, entry_stat.st_size, offset))
    {
        return EXIT_FAILURE;
    }
        

    munmap(target_buf, target_stat.st_size);
    close(target_fd);
    
}
