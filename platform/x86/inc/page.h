#ifndef _PAGE_H
#define _PAGE_H
#include <stdint.h>
/* Common page attribute bit definitions */
#define PG_PRESENT  0x001  /* Present in memory */
#define PG_RW       0x002  /* Read/Write (1=RW, 0=Read-only) */
#define PG_USER     0x004  /* User access (1=User, 0=Supervisor) */
#define PG_PWT      0x008  /* Write-through */
#define PG_PCD      0x010  /* Cache disabled */
#define PG_ACCESSED 0x020  /* Accessed */
#define PG_DIRTY    0x040  /* Dirty (PTE only) */
#define PG_EXEC     0x080  /* Executable */

#define PAGE_SIZE   0x1000 /* 4KB page size */


typedef uint32_t pde_t;
typedef uint32_t pte_t;
typedef uint32_t phys_addr_t;  /* Represents a physical address value (numeric only, not directly dereferenceable). */
typedef uint32_t virt_addr_t;  /* Represents a virtual address. */
void init_paging(void);
void flush_tlb(uint32_t virtual_addr);
pde_t* get_page_directory(void);
pte_t* get_page_table(void);
pte_t* get_page_table2(void);
#define KERNEL_VIRT_START 0xC0000000
#define KERNEL_VIRT_END   0xFFFFFFFF
#define KERNEL_OFFSET     KERNEL_VIRT_START

/* Extract the upper 20 bits of an address (aligned to 4KB) */
#define PAGE_ADDR(addr) ((uint32_t)(addr) & 0xFFFFF000)
/* Define the starting kernel page-directory index (0xC0000000 >> 22 = 768). */
#define KERNEL_PDE_START 768

__attribute__((section(".boot.text")))
void load_page_directory(uint32_t pd);

__attribute__((section(".boot.text")))
void enable_paging(void);

#endif /* _PAGE_H */
