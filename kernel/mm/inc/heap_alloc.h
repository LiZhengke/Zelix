#ifndef HEAP_ALLOC_H
#define HEAP_ALLOC_H

#include <stdint.h>
#include "mmu.h"

void  kmalloc_init(pde_t *page_dir_virt, uint32_t initial_pages);
void *kmalloc(uint32_t size);
void  kfree(void *ptr);

#endif /* HEAP_ALLOC_H */
