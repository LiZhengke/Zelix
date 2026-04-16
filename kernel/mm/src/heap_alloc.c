#include <stdint.h>
#include <stddef.h>
#include "mmu.h"
#include "pmm.h"

typedef struct header {
    uint32_t size;        /* Block size (including this header) */
    uint32_t is_free;     /* Whether this block is free */
    struct header *next;  /* Pointer to the next block */
} header_t;

static header_t *heap_start = NULL;

void kmalloc_init(pde_t *page_dir_virt, uint32_t initial_pages) {
    /* 1. Obtain contiguous virtual space using vmm_alloc and map_page */
    heap_start = (header_t *)kernel_malloc_page(page_dir_virt, initial_pages);

    /* 2. Initialize the first large free block */
    heap_start->size = initial_pages * PAGE_SIZE;
    heap_start->is_free = 1;
    heap_start->next = NULL;
}

void* kmalloc(uint32_t size) {
    uint32_t total_size = size + sizeof(header_t);
    header_t *curr = heap_start;

    while (curr) {
        if (curr->is_free && curr->size >= total_size) {
            /* Found a suitable block! */

            /* Split this block if there is enough remaining space */
            if (curr->size > total_size + sizeof(header_t) + 4) {
                header_t *next_block = (header_t *)((uint32_t)curr + total_size);
                next_block->size = curr->size - total_size;
                next_block->is_free = 1;
                next_block->next = curr->next;

                curr->size = total_size;
                curr->next = next_block;
            }

            curr->is_free = 0;
            /* Return pointer to the data area (past the header) */
            return (void *)((uint32_t)curr + sizeof(header_t));
        }
        curr = curr->next;
    }

    /* No suitable block found; should expand the heap or report an error */
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr) return;

    /* Walk back to the header location */
    header_t *header = (header_t *)((uint32_t)ptr - sizeof(header_t));
    header->is_free = 1;

    /* Simple defragmentation: merge adjacent free blocks */
    header_t *curr = heap_start;
    while (curr && curr->next) {
        if (curr->is_free && curr->next->is_free) {
            curr->size += curr->next->size;
            curr->next = curr->next->next;
        } else {
            curr = curr->next;
        }
    }
}
