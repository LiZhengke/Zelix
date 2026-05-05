#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "ff.h"
#include "elf.h"
#include "mmu.h"
#include "loader.h"

#define LOAD_ADDR 0x400000
#define ELF_PF_X 0x1U
#define ELF_PF_W 0x2U
#define ELF_PF_R 0x4U

static uint32_t loader_get_mapped_phys(pde_t* pgd, uint32_t va)
{
    uint32_t pde = pgd[va >> 22];
    if ((pde & PG_PRESENT) == 0)
        return 0;

    uint32_t *pt = (uint32_t *)p2v((phys_addr_t)(pde & 0xFFFFF000U));
    uint32_t pte = pt[(va >> 12) & 0x3FFU];
    if ((pte & PG_PRESENT) == 0)
        return 0;

    return (pte & 0xFFFFF000U);
}

static bool loader_map_missing_range(pde_t *pgd, uint32_t va_start, uint32_t va_end)
{
    uint32_t va;

    for (va = va_start & 0xFFFFF000U; va < va_end; va += PAGE_SIZE) {
        if (loader_get_mapped_phys(pgd, va) != 0)
            continue;

        if (map_user_code(pgd, (void *)va, PAGE_SIZE) == 0)
            return false;
    }

    return true;
}

static bool loader_apply_segment_flags(pde_t *pgd, uint32_t va_start, uint32_t va_end, uint32_t seg_flags)
{
    uint32_t add_flags = PG_USER;
    uint32_t va;

    (void)ELF_PF_R;

    if (seg_flags & ELF_PF_W)
        add_flags |= PG_RW;
    if (seg_flags & ELF_PF_X)
        add_flags |= PG_EXEC;

    for (va = va_start & 0xFFFFF000U; va < va_end; va += PAGE_SIZE) {
        uint32_t pde = pgd[va >> 22];
        uint32_t *pt;
        uint32_t idx;
        uint32_t pte;

        if ((pde & PG_PRESENT) == 0)
            return false;

        pt = (uint32_t *)p2v((phys_addr_t)(pde & 0xFFFFF000U));
        idx = (va >> 12) & 0x3FFU;
        pte = pt[idx];
        if ((pte & PG_PRESENT) == 0)
            return false;

        pt[idx] = (pte & 0xFFFFF000U) | ((pte & 0xFFFU) | add_flags) | PG_PRESENT;
        flush_tlb(va);
    }

    return true;
}

static bool loader_zero_user_range(pde_t *pgd, uint32_t va_start, uint32_t size)
{
    uint32_t va = va_start;
    uint32_t remain = size;

    while (remain > 0) {
        uint32_t phys = loader_get_mapped_phys(pgd, va);
        uint32_t off = va & (PAGE_SIZE - 1U);
        uint32_t chunk;
        unsigned char *dst;

        if (phys == 0)
            return false;

        chunk = PAGE_SIZE - off;
        if (chunk > remain)
            chunk = remain;

        dst = (unsigned char *)p2v((phys_addr_t)phys) + off;
        memset(dst, 0, chunk);

        va += chunk;
        remain -= chunk;
    }

    return true;
}

static bool loader_read_user_range(FIL *file, pde_t *pgd, uint32_t va_start, uint32_t size)
{
    uint32_t va = va_start;
    uint32_t remain = size;

    while (remain > 0) {
        uint32_t phys = loader_get_mapped_phys(pgd, va);
        uint32_t off = va & (PAGE_SIZE - 1U);
        uint32_t chunk;
        unsigned char *dst;
        UINT br = 0;
        FRESULT res;

        if (phys == 0)
            return false;

        chunk = PAGE_SIZE - off;
        if (chunk > remain)
            chunk = remain;

        dst = (unsigned char *)p2v((phys_addr_t)phys) + off;
        res = f_read(file, dst, chunk, &br);
        if (res != FR_OK || br != chunk)
            return false;

        va += chunk;
        remain -= chunk;
    }

    return true;
}

void *elf_load(const char *path, pde_t* pgd, bool check_only)
{
    FIL file;
    FRESULT res;
    UINT br;
    uint16_t i;

    Elf32_Ehdr ehdr;

    /* 打开文件 */
    res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        printf("open ELF failed (%d)\n", res);
        return NULL;
    }

    /* 读取 ELF header */
    res = f_read(&file, &ehdr, sizeof(ehdr), &br);
    if (res != FR_OK || br != sizeof(ehdr)) {
        printf("read ehdr failed (%d)\n", res);
        return NULL;
    }

    /* 校验 ELF */
    if (ehdr.e_ident[0] != 0x7f ||
        ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' ||
        ehdr.e_ident[3] != 'F') {
        printf("not ELF\n");
        return NULL;
    }

    printf("ELF entry = 0x%x\n", ehdr.e_entry);
    printf("ELF Number of program headers = %d\n", ehdr.e_phnum);
    printf("ELF Program header offset = %d\n", ehdr.e_phoff);

    /* 遍历 program header */
    for (i = 0; i < ehdr.e_phnum; i++) {
        Elf32_Phdr phdr;

        /* 定位 phdr */
        f_lseek(&file, ehdr.e_phoff + ( uint32_t ) i * sizeof(phdr));

        f_read(&file, &phdr, sizeof(phdr), &br);

        if (phdr.p_type != PT_LOAD)
            continue;

        printf("#%d LOAD seg: vaddr=0x%x size=%u filesz=%u offset=%u\n",
                i, phdr.p_vaddr, phdr.p_memsz, phdr.p_filesz, phdr.p_offset);

        uint32_t va_start = phdr.p_vaddr & 0xFFFFF000U;
        uint32_t va_end   = (phdr.p_vaddr + phdr.p_memsz + PAGE_SIZE - 1U) & 0xFFFFF000U;
        uint32_t size     = va_end - va_start;

        printf("Mapping user code: va_start=0x%x va_end=0x%x size=%u\n", va_start, va_end, size);
        if (check_only || pgd == NULL) {
            printf("Check complete.\n");
            continue;
        }

        printf("Mapping user code...\n");
        if (!loader_map_missing_range(pgd, va_start, va_end)) {
            printf("map user code failed\n");
            f_close(&file);
            return NULL;
        }

        printf("Applying segment flags...\n");
        if (!loader_apply_segment_flags(pgd, va_start, va_end, phdr.p_flags)) {
            printf("set segment flags failed\n");
            f_close(&file);
            return NULL;
        }

        printf("Loading segment data...\n");
        f_lseek(&file, phdr.p_offset);
        if (phdr.p_filesz > 0 && !loader_read_user_range(&file, pgd, phdr.p_vaddr, phdr.p_filesz)) {
            printf("read segment failed\n");
            f_close(&file);
            return NULL;
        }

        printf("Zeroing BSS if needed...\n");
        if (phdr.p_memsz > phdr.p_filesz &&
            !loader_zero_user_range(pgd, phdr.p_vaddr + phdr.p_filesz, phdr.p_memsz - phdr.p_filesz)) {
            printf("zero bss failed\n");
            f_close(&file);
            return NULL;
        }
    }

    f_close(&file);

    return (void*)ehdr.e_entry;
}

void *elf_exec(const char *path, pde_t* pgd)
{
    FIL file;
    FRESULT res;
    UINT br;
    uint16_t i;

    Elf32_Ehdr ehdr;

    /* 打开文件 */
    res = f_open(&file, path, FA_READ);
    if (res != FR_OK) {
        printf("open ELF failed (%d)\n", res);
        return NULL;
    }

    /* 读取 ELF header */
    res = f_read(&file, &ehdr, sizeof(ehdr), &br);
    if (res != FR_OK || br != sizeof(ehdr)) {
        printf("read ehdr failed (%d)\n", res);
        return NULL;
    }

    /* 校验 ELF */
    if (ehdr.e_ident[0] != 0x7f ||
        ehdr.e_ident[1] != 'E' ||
        ehdr.e_ident[2] != 'L' ||
        ehdr.e_ident[3] != 'F') {
        printf("not ELF\n");
        return NULL;
    }

    printf("ELF entry = 0x%x\n", ehdr.e_entry);
    printf("ELF Number of program headers = %d\n", ehdr.e_phnum);
    printf("ELF Program header offset = %d\n", ehdr.e_phoff);

    /* vTaskExec(); */
    free_user_space(pgd);
    /* 遍历 program header */
    for (i = 0; i < ehdr.e_phnum; i++) {
        Elf32_Phdr phdr;
        unsigned char *dest = NULL;

        /* 定位 phdr */
        f_lseek(&file, ehdr.e_phoff + ( uint32_t ) i * sizeof(phdr));

        f_read(&file, &phdr, sizeof(phdr), &br);

        if (phdr.p_type != PT_LOAD)
            continue;

        printf("#%d LOAD seg: vaddr=0x%x size=%u filesz=%u offset=%u\n",
                i, phdr.p_vaddr, phdr.p_memsz, phdr.p_filesz, phdr.p_offset);

        dest = (unsigned char *)map_user_code( pgd, ( void * ) phdr.p_vaddr, phdr.p_memsz );
        f_lseek(&file, phdr.p_offset);

        if(!dest)
            dest = (unsigned char *)phdr.p_vaddr;

        f_read(&file, dest, phdr.p_filesz, &br);

        /* BSS 清零 */
        if (phdr.p_memsz > phdr.p_filesz) {
            memset(dest + phdr.p_filesz, 0,
                    phdr.p_memsz - phdr.p_filesz);
        }
    }

    f_close(&file);

    return (void*)ehdr.e_entry;
}
