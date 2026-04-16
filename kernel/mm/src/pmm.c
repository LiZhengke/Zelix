#include <stddef.h>
#include "pmm.h"

uint8_t pmm_bitmap[BITMAP_SIZE];
/**
 * Allocate contiguous physical pages.
 * @param num_pages Number of contiguous pages to allocate
 * @return Starting address of the allocated physical pages, or NULL if insufficient contiguous pages
 */
void* pmm_alloc_page(size_t num_pages) {
    uint32_t consecutive_free = 0;
    uint32_t start_page = 0;

    /* Handle edge cases */
    if (num_pages == 0 || num_pages > TOTAL_PAGES) {
        return NULL;
    }

    /* Search for num_pages consecutive free pages */
    for (uint32_t i = 0; i < TOTAL_PAGES; i++) {
        if (!bitmap_test(i)) {
            /* This page is free */
            if (consecutive_free == 0) {
                start_page = i;  /* Remember the start of this potential block */
            }
            consecutive_free++;

            /* Check if we found enough consecutive pages */
            if (consecutive_free == num_pages) {
                /* Mark all pages as allocated */
                for (uint32_t j = 0; j < num_pages; j++) {
                    bitmap_set(start_page + j);
                }
                return (void*)(start_page * PAGE_SIZE);
            }
        } else {
            /* This page is occupied, reset the counter */
            consecutive_free = 0;
        }
    }

    return NULL; /* Out of memory or insufficient contiguous pages */
}

/**
 * Free a single physical page.
 */
void pmm_free_page(void* phys_addr) {
    uint32_t page_idx = (uint32_t)phys_addr / PAGE_SIZE;
    bitmap_unset(page_idx);
}

/**
 * Free contiguous physical pages.
 * @param phys_addr Starting address of the pages to free
 * @param num_pages Number of pages to free
 */
void pmm_free_pages(void* phys_addr, size_t num_pages) {
    uint32_t start_page = (uint32_t)phys_addr / PAGE_SIZE;

    for (uint32_t i = 0; i < num_pages; i++) {
        bitmap_unset(start_page + i);
    }
}

void pmm_init(uint32_t mem_size) {
    /* 1. Mark all pages as occupied by default */
    memset(pmm_bitmap, 0xFF, BITMAP_SIZE);

    /* 2. Mark pages in the usable region as free.
     *    Assumes memory from 4MB up to mem_size is available. */
    uint32_t start_page = 0x400000 / PAGE_SIZE;
    uint32_t end_page = mem_size / PAGE_SIZE;

    for (uint32_t i = start_page; i < end_page; i++) {
        bitmap_unset(i);
    }

    /* 3. Protect pages occupied by the kernel itself.
     *    Uses the _kernel_end linker symbol to determine the boundary. */
    extern char _kernel_end[];
    uint32_t kernel_pages_end = ((uint32_t)_kernel_end - 0xC0000000) / PAGE_SIZE;
    for (uint32_t i = start_page; i <= kernel_pages_end; i++) {
        bitmap_set(i);
    }
}
