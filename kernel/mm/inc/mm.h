#pragma once
#include <stdint.h>
#include "page.h"

struct task;
typedef struct
{
    phys_addr_t  pgd_phys;       /**< Page directory physical address */
    uint32_t *pgd;       /**< Page directory virtual address */

    uint32_t start_code, end_code;
    uint32_t start_data, end_data;
    uint32_t start_bss, end_bss;
    uint32_t  user_stack_top; /**< Top of user stack. */

    uint32_t brk;            /**< Current top of the heap (used by the sbrk syscall) */
    int count;               /**< Reference count (supports multiple threads sharing the same address space) */

} mm_struct;

mm_struct* mm_create(struct task * t);
void destroy_mm(mm_struct *mm);
