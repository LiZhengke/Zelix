/* Page directory / page table entry common attribute bits */
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "mmu.h"
#include "pmm.h"
#include "vmm.h"
#include "heap_alloc.h"
#include "portmacro.h"

/* Extract the upper 20 bits of an address (aligned to 4KB) */
#define PAGE_ADDR(addr) ((uint32_t)(addr) & 0xFFFFF000)
/* Define the starting kernel page-directory index (0xC0000000 >> 22 = 768). */
#define KERNEL_PDE_START 768


#if ( configRUN_ADDITIONAL_TESTS == 1 )
    #define MMU_TEST_ASSERT( x )                                                                 \
    do                                                                                            \
    {                                                                                             \
        if( ( x ) == 0 )                                                                          \
        {                                                                                         \
            printf( "[MMU TEST] FAIL: %s\n", #x );                                               \
            for( ; ; )                                                                            \
            {                                                                                     \
            }                                                                                     \
        }                                                                                         \
    } while( 0 )


    static void mmu_test() {
        uint32_t test_virt = 0xDEADC000; // Virtual address.
        uint32_t test_phys = 0x2000000; // Physical address at 32MB.
        uint32_t* page_dir_virt = (uint32_t *)p2v((phys_addr_t)page_directory); // Get virtual address of the first page directory.
        map_page(page_dir_virt, test_virt, test_phys, PG_PRESENT | PG_RW | PG_USER);
        /*load_page_directory((uint32_t)page_directory);*/ /* Refresh TLB. */

        // Try writing.
        volatile uint32_t *ptr = (uint32_t*)test_virt;
        *ptr = 0x12345678;

        MMU_TEST_ASSERT( *ptr == 0x12345678 ); /* Verify the value is correctly written and read back. */
    }

    static void cpuid_test(uint32_t eax_in) {
        uint32_t eax, ebx, ecx, edx;
        __asm__ volatile (
            "cpuid"
            : "=a" (eax), "=b" (ebx), "=c" (ecx), "=d" (edx)
            : "a" (eax_in)
        );
        switch (eax_in)
        {
        case 0:
            char vendor[13];
            memcpy(vendor, &ebx, 4);
            memcpy(vendor + 4, &edx, 4);
            memcpy(vendor + 8, &ecx, 4);
            vendor[12] = '\0';
            printf("CPU Vendor: %s\n", vendor);
            break;
        case 1:
            printf("CPU Family: %s, Model: %s, Stepping: %d\n",
                ((eax >> 8) & 0xF) == 4 ? "486" : ((eax >> 8) & 0xF) == 5 ? "Pentium" : "Unknown",
                ((eax >> 4) & 0xF) == 0 ? "DX" : ((eax >> 4) & 0xF) == 1 ? "SX" : "Other",
                eax & 0xF);
            printf("CPU Features - FPU: %d, VME: %d, DE: %d, PSE: %d, TSC: %d, MSR: %d, PAE: %d, MCE: %d, \
                CX8: %d,APIC: %d,MTRR: %d,SEP: %d,RR: %d,PGE: %d\n",
                (edx >> 0) & 1, (edx >> 1) & 1, (edx >> 2) & 1, (edx >> 3) & 1,
                (edx >> 4) & 1, (edx >> 5) & 1, (edx >> 6) & 1, (edx >> 7) & 1,
                (edx >> 8) & 1, (edx >> 9) & 1, (edx >> 10) & 1, (edx >> 11) & 1,
                (edx >> 12) & 1, (edx >> 13) & 1);

        break;
        default:
            break;
        }

        // printf("CPU EAX: 0x%08x, EBX: 0x%08X, ECX: 0x%08X, EDX: 0x%08X\n", eax, ebx, ecx, edx);
    }

    static void prvRunMmuUnitTests( void )
    {
        uint32_t testPhys = 0;
        uint32_t * testPgd = NULL;
        uint32_t testVirt = 0x00403000U;
        uint32_t * testPt = NULL;
        uint32_t pde = 0;
        uint32_t pte = 0;

        printf( "[MMU TEST] start\n" );

        create_user_page_directory( &testPhys, &testPgd );
        MMU_TEST_ASSERT( testPhys != 0U );
        MMU_TEST_ASSERT( testPgd != NULL );

        MMU_TEST_ASSERT( testPgd[ 0 ] == page_directory[ 0 ] );
        MMU_TEST_ASSERT( testPgd[ KERNEL_PDE_START ] == page_directory[ KERNEL_PDE_START ] );
        MMU_TEST_ASSERT( testPgd[ 1023 ] == page_directory[ 1023 ] );
        MMU_TEST_ASSERT( testPgd[ 1 ] == 0U );

        map_page( testPgd, testVirt, 0x00123000U, PG_RW | PG_USER );

        pde = testPgd[ testVirt >> 22 ];
        MMU_TEST_ASSERT( ( pde & PG_PRESENT ) != 0U );
        testPt = ( uint32_t * ) p2v( pde & 0xFFFFF000U );
        pte = testPt[ ( testVirt >> 12 ) & 0x3FFU ];
        MMU_TEST_ASSERT( ( pte & 0xFFFFF000U ) == 0x00123000U );
        MMU_TEST_ASSERT( ( pte & ( PG_PRESENT | PG_RW | PG_USER ) ) == ( PG_PRESENT | PG_RW | PG_USER ) );

        MMU_TEST_ASSERT( user_to_phys( ( void * ) 0x08048000U ) == 0x08048000U );

        mmu_test(); /* Run a simple mapping test to ensure MMU works correctly. */
        cpuid_test(0); /* Run a simple CPUID test to verify CPU vendor string retrieval. */
        cpuid_test(1); /* Run CPUID with EAX=1 to get feature information. */
        printf( "[MMU TEST] PASS\n" );
    }
#endif


void map_page(uint32_t *dir_vir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t pd_index = virtual_addr >> 22;            /* Upper 10 bits */
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;  /* Middle 10 bits */

    /* Check whether the corresponding page table exists */
    if (!dir_vir[pd_index] || !(dir_vir[pd_index] & PG_PRESENT)) {
        /* Page table does not exist; allocate a physical page for it.
         * Note: this must be a physical page address! */
        uint32_t new_pt = (uint32_t)pmm_alloc_page(1); /* Allocate one page for the page table */
        if (!new_pt) return; /* Out of memory - in a real kernel, you'd want to handle this more gracefully */

        /* Zero-fill the new page table to prevent stale mappings */
        uint32_t *pt_ptr = (uint32_t *)p2v(new_pt); /* Convert to virtual address for initialization */
        for(int i = 0; i < 1024; i++) pt_ptr[i] = 0;

        /* Set the PDE with full permissions; fine-grained access is controlled by the PTE */
        dir_vir[pd_index] = new_pt | PG_PRESENT | PG_RW | PG_USER;
    }
    /*
        * At this point, the page table exists. Get its physical address from the PDE,
        * convert to virtual address, and set the PTE for the desired mapping.
        */
    uint32_t pt_phys_addr = dir_vir[pd_index] & 0xFFFFF000;
    uint32_t *page_table = (uint32_t *)p2v(pt_phys_addr);

    /* Set the PTE with the physical address and flags */
    page_table[pt_index] = (physical_addr & 0xFFFFF000) | flags | PG_PRESENT;

    /* Flush TLB so the CPU does not use a stale mapping */
    flush_tlb(virtual_addr);
}

void create_user_page_directory(uint32_t* pgd_phys, uint32_t** pgd_virt) {
    /* Allocate a physical page for the new page directory */
    uint32_t new_pd_phys = (uint32_t)pmm_alloc_page(1); /* Allocate one page for the page directory */
    if (!new_pd_phys) return;

    uint32_t *pd = (uint32_t *)p2v(new_pd_phys); /* Convert to virtual address for initialization */

    /* Clear user-space entries (0 - 767) */
    for (int i = 0; i < KERNEL_PDE_START; i++) {
        pd[i] = 0;
    }
    pde_t *boot_pgd_virt = (pde_t *)p2v((phys_addr_t)get_page_directory()); /* Get virtual address of the boot page directory. */
    /* Copy kernel-space entries (768 - 1023) from the boot page directory */
    for (int i = KERNEL_PDE_START; i < 1024; i++) {
        pd[i] = boot_pgd_virt[i];
    }

    pd[0] = boot_pgd_virt[0]; /* Identity map the first 4MB for user-space code */

    if (pgd_phys) *pgd_phys = new_pd_phys;
    if (pgd_virt) *pgd_virt = pd;

}

void* kernel_malloc_page(pde_t* page_dir_virt, size_t pages) {
    /* 1. Find a free region in virtual memory */
    void* virt_addr = vmm_alloc(pages);
    if (!virt_addr) return NULL;

    for (size_t i = 0; i < pages; i++) {
        /* 2. Allocate a physical RAM page */
        uint32_t phys_addr = (uint32_t)pmm_alloc_page(1); /* Allocate one page */

        /* 3. Establish the virtual-to-physical mapping */
        uint32_t current_v = (uint32_t)virt_addr + (i * PAGE_SIZE);
        map_page(page_dir_virt, current_v, phys_addr, PG_PRESENT | PG_RW);
    }

    return virt_addr;
}

/* Symbols exported by the linker script. */
// Logic to compute physical addresses for user template sections.
uint32_t user_to_phys(void *v_addr) {
/*    uint32_t virt = (uint32_t)v_addr;
    uint32_t v_start = (uint32_t)_user_text_vma_start;
    uint32_t p_start = (uint32_t)_kernel_phys_end;

    // Physical address = physical base + (virtual address - virtual base)
    return p_start + (virt - v_start);
    */
   return (uint32_t)v_addr;
}

void map_user_section(pde_t* pgd, void* user_stack_top, size_t user_stack_depth, const char * const pcName) {
     /* Compute stack size in bytes. */
    uint32_t stack_size = user_stack_depth * sizeof( StackType_t );
    /* Allocate physical pages for the user stack. */
    uint32_t user_stack_phys = (uint32_t)pmm_alloc_page(stack_size / 4096);

    /* Compute the user-stack virtual address. */
    uint32_t user_stack_virt = (uint32_t)user_stack_top;
    /* Compute how many pages the user stack occupies. */
    /* Assumes STACK_SIZE is a multiple of 4096. */
    uint32_t num_pages = (stack_size + 4095) / 4096;
    size_t i;

    /* Map enough pages according to STACK_SIZE. */
    for( i = 0; i < num_pages; i++ )
    {
        /* Mapping logic:
        * Physical page: user_stack_phys + (i * 4096)
        * Virtual page:  (user_stack_virt - stack_size) + (i * 4096)
        * Note: stack-top is typically the end of the page range.
        */
        uint32_t phys_page = user_stack_phys + (i * 4096);
        uint32_t virt_page = (user_stack_virt - stack_size) + (i * 4096);
        map_page( pgd,
                virt_page,
                phys_page,
                PG_PRESENT | PG_RW | PG_USER );
    }
}

uint32_t spawn_user_task(pde_t* pgd,void* user_entry,size_t user_task_section_size) {
    // Get the cached user program image location inside the kernel.
    uint32_t prog_phys = (uint32_t)pmm_alloc_page(user_task_section_size/4096);
    uint32_t user_entry_addr = ( uint32_t ) user_entry;

    uint32_t num_pages = (user_task_section_size + 4095) / 4096;

    size_t i;

    /* Map enough pages according to STACK_SIZE. */
    for( i = 0; i < num_pages; i++ )
    {
        uint32_t phys_page = prog_phys  + (i * 4096);
        uint32_t virt_page = user_entry_addr + (uint32_t)(i * 4096);
        map_page( pgd,
                virt_page,
                phys_page,
                PG_PRESENT | PG_RW | PG_USER );
    }

    return prog_phys;
}

void mmu_init(void) {
    pmm_init(MEMORY_MAX_SIZE); /* Initialize the physical memory manager (assume 128MB total RAM). */
    kmalloc_init(p2v((phys_addr_t)get_page_directory()), 16); /* Initialize kernel heap, preallocating 16 pages (64KB). */

    #if ( configRUN_ADDITIONAL_TESTS == 1 )
        prvRunMmuUnitTests();
    #endif
}

/*
    * Free all user-space pages with PG_EXEC flag in the given page directory.
    * This is typically called when a user task exits to clean up its memory.
    * It iterates through the user-space PDEs and their corresponding PTEs,
    * freeing any physical pages that are marked as present and user-accessible.
*/
void free_user_space(uint32_t *pgd) {
    uint32_t *pgdir = pgd;

    for (int i = 0; i < KERNEL_PDE_START; i++) {
        uint32_t pde = pgdir[i];

        // 跳过不存在的
        if (!(pde & PG_PRESENT))
            continue;

        uint32_t *pt = (uint32_t*)(pde & ~0xFFF);

        for (int j = 0; j < 1024; j++) {
            uint32_t pte = pt[j];

            if (!(pte & PG_PRESENT))
                continue;

            // 🔥 只释放用户页
            if (pte & PG_USER) {
                void *phys = (void*)(pte & ~0xFFF);
                pmm_free_pages(phys, 1);   // 释放物理页
            }
            pt[j] = 0;
        }
        // 🔥 释放页表本身
        pmm_free_pages(pt, 1);
        pgdir[i] = 0;
    }
}
