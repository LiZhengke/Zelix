#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "ff.h"
#include "elf.h"
#include "mmu.h"
#include "loader.h"

#define LOAD_ADDR 0x400000

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
        unsigned char *dest = NULL;

        /* 定位 phdr */
        f_lseek(&file, ehdr.e_phoff + ( uint32_t ) i * sizeof(phdr));

        f_read(&file, &phdr, sizeof(phdr), &br);

        if (phdr.p_type != PT_LOAD)
            continue;

        printf("#%d LOAD seg: vaddr=0x%x size=%u filesz=%u offset=%u\n",
                i, phdr.p_vaddr, phdr.p_memsz, phdr.p_filesz, phdr.p_offset);

        if(!check_only && pgd)
            dest = (unsigned char *)map_user_code( pgd, ( void * ) phdr.p_vaddr, phdr.p_memsz );
        else
            printf("Check complete.\n");

        f_lseek(&file, phdr.p_offset);

        if(!dest)
            dest = (unsigned char *)phdr.p_vaddr;

        if(!check_only && pgd)
            f_read(&file, dest, phdr.p_filesz, &br);
        else
            f_lseek(&file, phdr.p_filesz);

        /* BSS 清零 */
        if(!check_only && pgd)
            if (phdr.p_memsz > phdr.p_filesz) {
                memset(dest + phdr.p_filesz, 0,
                        phdr.p_memsz - phdr.p_filesz);
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
