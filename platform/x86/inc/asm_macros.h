#ifndef __ASM_MACROS_H__
#define __ASM_MACROS_H__

/* -----------------------------

 * Segment registers
 * ----------------------------- */
#define KERNEL_CS 0x08
#define KERNEL_DS 0x10

/* -------------------------
 * Save general purpose registers
 * For 486 (no SSE)
 * ----------------------------- */
#define PUSH_GPRS()        \
    pushl %eax;            \
    pushl %ebx;            \
    pushl %ecx;            \
    pushl %edx;            \
    pushl %esi;            \
    pushl %edi;            \
    pushl %ebp

#define POP_GPRS()         \
    popl %ebp;             \
    popl %edi;             \
    popl %esi;             \
    popl %edx;             \
    popl %ecx;             \
    popl %ebx;             \
    popl %eax

/* -----------------------------
 * Save segment registers
 * Must be explicitly saved on 486
 * ----------------------------- */
#define PUSH_SEGS()        \
    pushl %ds;             \
    pushl %es;             \
    pushl %fs;             \
    pushl %gs

#define POP_SEGS()         \
    popl %gs;              \
    popl %fs;              \
    popl %es;              \
    popl %ds

/* -------------------------
 * Complete context
 * ----------------------------- */
#define SAVE_CONTEXT()     \
    PUSH_SEGS();           \
    PUSH_GPRS()

#define RESTORE_CONTEXT()  \
    POP_GPRS();            \
    POP_SEGS()

#endif
