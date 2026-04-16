#include <stdio.h>
#include <string.h>
#include "tss.h"
/**
 * @brief Generates a documentation comment for the selected code.
 *
 * This function analyzes the provided code selection and creates appropriate
 * documentation comments based on the code structure, parameters, return values,
 * and functionality.
 *
 * @param selection Pointer to the code selection that needs to be documented
 * @param format The documentation format to use (e.g., JSDoc, Doxygen, etc.)
 * @param options Additional options for customizing the documentation output
 *
 * @return A pointer to the generated documentation string, or NULL on failure
 *
 * @note The caller is responsible for freeing the returned documentation string
 * @note This function does not modify the original code selection
 *
 * @warning Ensure the selection pointer is valid before calling this function
 *
 * @see parse_code_structure()
 * @see format_documentation()
 *
 * @example
 * char* docs = generate_documentation(code_selection, "doxygen", NULL);
 * if (docs != NULL) {
 *     printf("%s\n", docs);
 *     free(docs);
 * }
 */
/**
 * @brief Task State Segment (TSS) structure for i486 architecture.
 *
 * The TSS is a special data structure used by the x86 processor to store
 * information about a task. It contains the processor register state and
 * other information needed for task switching. In protected mode, each
 * task can have its own TSS which the processor uses during context switches.
 *
 * This structure is used by the FreeRTOS kernel for the i486 flat memory
 * model port to manage hardware task switching and maintain task context.
 */
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

/**
 * @brief Initializes the Task State Segment (TSS) for i486 architecture.
 *
 * Sets up the TSS with the kernel stack pointer and disables I/O port access
 * by default. The TSS is zeroed out and then configured with the kernel mode
 * stack information.
 *
 * @param kernel_stack_top The top of the kernel stack for privilege level 0
 */
void init_tss(uint32_t kernel_stack_top)
{
    memset(&tss_entry, 0, sizeof(tss_entry));

    // Set kernel mode (ring 0) stack segment selector
    tss_entry.ss0 = KERNEL_DS;
    // Set kernel mode (ring 0) stack pointer to top of kernel stack
    tss_entry.esp0 = kernel_stack_top;

    /**
     * @brief I/O bitmap: placed after the end of TSS = disable all I/O operations
     *
     * By positioning the I/O permission bitmap beyond the TSS limit, all I/O port
     * access is effectively disabled for this task, as the processor cannot find
     * valid permission bits.
     */
    tss_entry.iomap_base = sizeof(struct tss);
}

/**
 * @brief Loads the Task State Segment (TSS) into the task register.
 *
 * This function executes the LTR (Load Task Register) instruction to load
 * the TSS selector into the processor's task register. This must be done
 * after the TSS has been initialized and its descriptor has been added to
 * the GDT. Once loaded, the processor will use this TSS for task switching
 * operations.
 *
 * @note This function must be called in kernel mode (ring 0)
 * @note The TSS descriptor must be present in the GDT before calling this
 */
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

uint32_t* tss_get_address(void)
{
    return (uint32_t*) &tss_entry;
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
