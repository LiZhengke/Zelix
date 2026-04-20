#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include "stdint.h"
#pragma once
#include <stdint.h>

/*
 * 与汇编保存顺序严格一致：
 * pushad + 段寄存器 + CPU 自动压栈
 */
struct trap_frame {
    /* 手动保存（汇编） */
    uint32_t gs, fs, es, ds;

    /* pushad 顺序（注意顺序） */
    uint32_t edi, esi, ebp, esp_dummy, ebx, edx, ecx, eax;

    /* CPU 自动压栈 */
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;

    /* 仅当从 ring3 进入时存在 */
    uint32_t esp;
    uint32_t ss;
}__attribute__((packed));

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
