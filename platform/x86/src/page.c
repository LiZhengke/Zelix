/* Page directory / page table entry common attribute bits */
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include "page.h"

#define configSUPPORT_PAGE_TABLE_TWO 1

/* Page directory must be aligned to a 4KB boundary */
__attribute__((section(".bootdata"), aligned(PAGE_SIZE)))
static pde_t page_directory[1024] __attribute__((aligned(PAGE_SIZE)));

/* First page table, used to map the first 4MB where the kernel resides */
__attribute__((section(".bootdata"), aligned(PAGE_SIZE)))
static pte_t page_table[1024] __attribute__((aligned(PAGE_SIZE)));

#if configSUPPORT_PAGE_TABLE_TWO == 1
/* Second page table, used to map the next 4MB where the kernel resides */
__attribute__((section(".bootdata"), aligned(PAGE_SIZE)))
static pte_t page_table2[1024] __attribute__((aligned(PAGE_SIZE)));
#endif

pde_t* get_page_directory(void) {
    return page_directory;
}

pte_t* get_page_table(void) {
    return page_table;
}

#if configSUPPORT_PAGE_TABLE_TWO == 1
pte_t* get_page_table2(void) {
    return page_table2;
}
#endif

void load_page_directory(uint32_t pd) {
    asm volatile ("movl %0, %%cr3" :: "r" (pd));
}

void enable_paging(void) {
    __asm volatile (
        "movl %%cr0, %%eax\n"
        "orl $0x80000000, %%eax\n"
        "movl %%eax, %%cr0\n"
        :
        :
        : "eax"
    );
}

__attribute__((section(".boot.text")))
void init_paging() {
    // 1. Initialize page-directory entries as not-present.
    for(int i = 0; i < 1024; i++) {
        page_directory[i] = 0 | PG_RW; // Mark not-present and writable.
    }

    // 2. Fill the first page table: map 0.0MB to 4.0MB (identity mapping).
    for(uint32_t i = 0; i < 1024; i++) {
        // Write physical address (i * 4KB) into the page table.
        page_table[i] = (i * 0x1000) | PG_PRESENT | PG_RW;
#if configSUPPORT_PAGE_TABLE_TWO == 1
        page_table2[i] = (i * 0x1000 + 0x400000) | PG_PRESENT | PG_RW;
#endif
    }

    // 3. Put page tables into the first page-directory entries.
    page_directory[0] = ((uint32_t)page_table) | PG_PRESENT | PG_RW;
#if configSUPPORT_PAGE_TABLE_TWO == 1
    page_directory[1] = ((uint32_t)page_table2) | PG_PRESENT | PG_RW;
#endif
    // 3.1 Map high-half virtual space to the same physical space (0xC0000000 -> 0x00000000).
    page_directory[KERNEL_PDE_START] = ((uint32_t)page_table) | PG_PRESENT | PG_RW;
#if configSUPPORT_PAGE_TABLE_TWO == 1
    page_directory[KERNEL_PDE_START + 1] = ((uint32_t)page_table2) | PG_PRESENT | PG_RW;
#endif
    // 4. Give the CPU the page-directory address (write CR3).
    load_page_directory((uint32_t)page_directory);

    // 5. Enable paging (set bit 31 in CR0).
    enable_paging();
}

/* Flush the TLB entry for a specific virtual address */
void flush_tlb(uint32_t virtual_addr) {
   __asm__ volatile("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}
