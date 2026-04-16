#ifndef PMM_H
#define PMM_H
#include <stdint.h>
#include "page.h"

#define MEMORY_MAX_SIZE (128 * 1024 * 1024) /* 128MB */
#define TOTAL_PAGES (MEMORY_MAX_SIZE / PAGE_SIZE)
#define BITMAP_SIZE (TOTAL_PAGES / 8)

extern uint8_t pmm_bitmap[BITMAP_SIZE];

/* Set a bit in the bitmap */
static inline void bitmap_set(uint32_t page_idx) {
    pmm_bitmap[page_idx / 8] |= (uint8_t)(1 << (page_idx % 8));
}

/* Clear a bit in the bitmap */
static inline void bitmap_unset(uint32_t page_idx) {
    pmm_bitmap[page_idx / 8] &= (uint8_t)~(1 << (page_idx % 8));
}

/* Test if a bit in the bitmap is set */
static inline int bitmap_test(uint32_t page_idx) {
    return pmm_bitmap[page_idx / 8] & (1 << (page_idx % 8));
}

void* pmm_alloc_page(size_t num_pages);
void pmm_free_page(void* phys_addr);
void pmm_free_pages(void* phys_addr, size_t num_pages);
void pmm_init(uint32_t mem_size);
#endif /* PMM_H */
