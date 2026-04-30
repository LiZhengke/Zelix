#ifndef MMU_H
#define MMU_H
#include <stdint.h>
#include <stddef.h>
#include "page.h"


void map_page(uint32_t *dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);

void* kernel_malloc_page(pde_t* page_dir_virt, size_t pages);
void create_user_page_directory(uint32_t* pgd_phys, uint32_t** pgd_virt);

/* Convert physical address to kernel virtual address (Physical to Virtual). */
static inline void* p2v(phys_addr_t phys) {
    /* This simple conversion is valid only for physical addresses in the 1MB ~ __phys_end range. */
    return (void *)(phys + KERNEL_OFFSET);
}

/* Convert kernel virtual address to physical address (Virtual to Physical). */
static inline phys_addr_t v2p(void* virt) {
    /* This conversion is valid only for kernel code/data virtual addresses. */
    return (phys_addr_t)((uint32_t)virt - KERNEL_OFFSET);
}

uint32_t user_to_phys(void *v_addr);
void map_user_stack(pde_t* pgd, void* user_stack_top, size_t user_stack_depth);
uint32_t map_user_code(pde_t* pgd,void* user_entry,size_t user_task_section_size);


void mmu_init(void);
void free_user_space(uint32_t *pgd);
#endif /* MMU_H */
