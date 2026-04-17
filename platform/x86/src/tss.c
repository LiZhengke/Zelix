#include <string.h>

#include "tss.h"

/* 32-bit x86 Task State Segment used for ring transitions. */
struct tss {
    uint32_t prev_tss;   // Previous TSS selector (used for hardware task switching)
    uint32_t esp0;       // Stack pointer for privilege level 0 (kernel mode)
    uint32_t ss0;        // Stack segment selector for privilege level 0
    uint32_t esp1;       // Stack pointer for privilege level 1 (rarely used)
    uint32_t ss1;        // Stack segment selector for privilege level 1
    uint32_t esp2;       // Stack pointer for privilege level 2 (rarely used)
    uint32_t ss2;        // Stack segment selector for privilege level 2
    uint32_t cr3;        // Page directory base register (physical address)
    uint32_t eip;        // Instruction pointer - next instruction to execute
    uint32_t eflags;     // Processor flags register
    uint32_t eax;        // General purpose register A (accumulator)
    uint32_t ecx;        // General purpose register C (counter)
    uint32_t edx;        // General purpose register D (data)
    uint32_t ebx;        // General purpose register B (base)
    uint32_t esp;        // Stack pointer register
    uint32_t ebp;        // Base pointer register (stack frame)
    uint32_t esi;        // Source index register
    uint32_t edi;        // Destination index register
    uint32_t es;         // Extra segment selector
    uint32_t cs;         // Code segment selector
    uint32_t ss;         // Stack segment selector
    uint32_t ds;         // Data segment selector
    uint32_t fs;         // Additional segment selector F
    uint32_t gs;         // Additional segment selector G
    uint32_t ldt;        // Local Descriptor Table selector
    uint16_t trap;       // Debug trap bit - generates debug exception on task switch
    uint16_t iomap_base; // Offset to I/O permission bitmap (from TSS base)
} __attribute__((packed));

static struct tss tss_entry;

void init_tss(uint32_t kernel_stack_top)
{
    memset(&tss_entry, 0, sizeof(tss_entry));

    tss_entry.ss0 = KERNEL_DS;
    tss_entry.esp0 = kernel_stack_top;

    /* No I/O bitmap is provided, so all port access is denied by default. */
    tss_entry.iomap_base = sizeof(struct tss);
}

void tss_load(void)
{
    __asm__ __volatile__ (
        "ltr %%ax"
        :
        : "a"(TSS_SELECTOR)
    );
}

uint32_t tss_get_size(void)
{
    return sizeof(struct tss);
}

uintptr_t tss_get_address(void)
{
    return (uintptr_t)&tss_entry;
}

void tss_set_esp0(uint32_t esp0)
{
    /*printf("tss_set_esp0: updating TSS esp0 to %p\n", (void*)esp0);*/
    tss_entry.esp0 = esp0;
}

uint32_t tss_get_esp0(void)
{
    return tss_entry.esp0;
}
