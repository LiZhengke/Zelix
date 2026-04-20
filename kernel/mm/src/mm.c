#include "mm.h"
#include "mmu.h"
#include "ktask.h"
#include "adapter.h"
#include "string.h"

mm_struct* mm_create(struct task * t) {
    /* 1. Allocate mm_struct from the kernel heap */
    mm_struct *mm = (mm_struct *)sched_task_alloc(sizeof(mm_struct));

    if( t->type == TASK_TYPE_PROCESS ) {
        memset(mm, 0, sizeof(mm_struct));

        /* 2. Create a new page directory with kernel mappings.
        *    This calls pmm_alloc_page() internally. */
        create_user_page_directory(&mm->pgd_phys, (uint32_t**)&mm->pgd); /* Returns physical address of new page directory */
    }else{
        /* For threads, share the same page directory as the current task. */
        mm->pgd_phys = t->mm->pgd_phys;
        mm->pgd = t->mm->pgd;
    }

    if (!mm->pgd_phys) {
        sched_task_free(mm);
        return NULL;
    }

    /* 3. Set defaults */
    mm->user_stack_top = t->user_stack_top; /* Below 3 GB as stack top */
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
