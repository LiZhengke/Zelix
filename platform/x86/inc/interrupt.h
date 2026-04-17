#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "stdint.h"

// Enhanced exception handler structure
// Captures all CPU registers and exception information during an interrupt/exception
struct exc_regs {
    // Segment registers
    uint32_t gs, fs, es, ds;

    // General purpose registers (saved by interrupt handler)
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;

    // Exception/interrupt vector number and error code
    uint32_t vector;      // Interrupt vector (32-47 for hardware IRQs)
    uint32_t err_code;    // Hardware error code (if applicable)

    // Instruction pointer and flags at time of exception
    uint32_t eip;         // Extended instruction pointer
    uint32_t cs;          // Code segment
    uint32_t eflags;      // Extended flags register

    // User-mode stack information
    uint32_t user_esp;    // User stack pointer (ring 3)
    uint32_t user_ss;     // User stack segment (ring 3)
};
#endif /* _INTERRUPT_H_ */
