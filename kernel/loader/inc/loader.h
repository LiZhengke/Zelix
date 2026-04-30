#ifndef LOADER_H
#define LOADER_H

#include <stdbool.h>

void *elf_load(const char *path, pde_t* pgd, bool check_only);
void *elf_exec(const char *path, pde_t* pgd);
#endif /* LOADER_H */
