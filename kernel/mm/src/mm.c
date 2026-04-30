#include <stdbool.h>
#include "mm.h"
#include "mmu.h"
#include "pmm.h"
#include "ktask.h"
#include "adapter.h"
#include "string.h"
#include "heap_alloc.h"
#include "loader.h"

mm_struct* mm_create(struct task * t, const char *name) {
    /* 1. Allocate mm_struct from the kernel heap */
    mm_struct *mm = (mm_struct *)sched_task_alloc(sizeof(mm_struct));

    if( t->type == TASK_TYPE_USER_PROCESS ) {
        memset(mm, 0, sizeof(mm_struct));

        /* 2. Create a new page directory with kernel mappings.
        *    This calls pmm_alloc_page() internally. */
        mm->user_stack_top = t->user_stack_top;
        create_user_page_directory(&mm->pgd_phys, (uint32_t**)&mm->pgd); /* Returns physical address of new page directory */
        map_user_stack(mm->pgd,
            ( uint32_t * ) mm->user_stack_top, t->user_stack_size);

        elf_load(name, mm->pgd, false); /* Check if the ELF file is valid and can be loaded. */

        // uint32_t entry_phys = map_user_code(mm->pgd, (void*)t->entry_virt, 1 /*TODO*/);
        // memcpy(p2v((phys_addr_t)entry_phys), (void*)t->entry, 200); /* Copy the user code into the allocated physical page. */
    }else{
        /* For threads, share the same page directory as the current task. */
        mm->pgd_phys = t->mm->pgd_phys;
        mm->pgd = t->mm->pgd;
    }

    if (!mm->pgd_phys) {
        sched_task_free(mm);
        return NULL;
    }

    mm->count = 1;

    return mm;
}

void destroy_mm(mm_struct *mm) {
    if (mm->pgd_phys) {
        /* Only free the page directory if this is a process (not a thread sharing the same pgd) */
        pmm_free_page(mm->pgd_phys);
    }
    sched_task_free(mm);
}
