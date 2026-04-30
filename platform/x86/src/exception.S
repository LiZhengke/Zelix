#include "asm_macros.h"

.section .text.exception
.code32
# Macro 1: Handle exceptions that don't generate error codes by hardware (e.g., divide by zero)
# These exceptions require a dummy error code to maintain a consistent stack layout
.macro ISR_NOERR vector
.global exc\vector
exc\vector:
    pushl $0           # Push dummy error code (0) since CPU didn't provide one
    pushl $\vector     # Push exception vector number as first argument to handler
    jmp common_stub    # Jump to common exception handling code
.endm

# Macro 2: Handle exceptions where hardware automatically pushes error code (e.g., GPF)
# The error code is already on the stack when the interrupt occurs
.macro ISR_ERR vector
.global exc\vector
exc\vector:
    # Error code is already on top of stack (pushed by CPU)
    pushl $\vector     # Push exception vector number as first argument to handler
    jmp common_stub    # Jump to common exception handling code
.endm

# --- Generate common exception handlers using macros ---
/* 0–7: Exceptions without error codes */
ISR_NOERR 0    /* #DE  Divide Error */
ISR_NOERR 1    /* #DB  Debug */
ISR_NOERR 2    /* NMI  Non-Maskable Interrupt */
ISR_NOERR 3    /* #BP  Breakpoint */
ISR_NOERR 4    /* #OF  Overflow */
ISR_NOERR 5    /* #BR  BOUND Range Exceeded */
ISR_NOERR 6    /* #UD  Invalid Opcode */
ISR_NOERR 7    /* #NM  Device Not Available */

/* 8–15: Mix of exceptions with/without error codes */
ISR_ERR   8    /* #DF  Double Fault (always error code) */
ISR_NOERR 9    /* Coprocessor Segment Overrun (reserved) */
ISR_ERR   10   /* #TS  Invalid TSS (error code) */
ISR_ERR   11   /* #NP  Segment Not Present (error code) */
ISR_ERR   12   /* #SS  Stack-Segment Fault (error code) */
ISR_ERR   13   /* #GP  General Protection Fault (error code) */
ISR_ERR   14   /* #PF  Page Fault (error code) */
ISR_NOERR 15   /* Reserved */

/* 16–31: Exceptions 16 and above */
ISR_NOERR 16   /* #MF  x87 FPU Floating-Point Error */
ISR_ERR   17   /* #AC  Alignment Check (error code) */
ISR_NOERR 18   /* #MC  Machine Check */
ISR_NOERR 19   /* #XM  SIMD Floating-Point Exception */
ISR_NOERR 20   /* Virtualization Exception */
ISR_NOERR 21   /* Control Protection Exception (CET, newer CPU) */
ISR_NOERR 22   /* Reserved */
ISR_NOERR 23   /* Reserved */
ISR_NOERR 24   /* Reserved */
ISR_NOERR 25   /* Reserved */
ISR_NOERR 26   /* Reserved */
ISR_NOERR 27   /* Reserved */
ISR_NOERR 28   /* Reserved */
ISR_NOERR 29   /* Reserved */
ISR_NOERR 30   /* #SX  Security Exception (newer CPU) */
ISR_NOERR 31   /* Reserved */
ISR_NOERR 32   /* First user-defined interrupt */
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47


# --- Unified handler logic for all exceptions ---
common_stub:
    PUSH_TRAP_FRAME()  /* Save all registers and segment selectors on stack */

 /*
     * Stack layout at this point (ESP → increases downward as we push):
     *
     *  [esp+0x00]  gs              (segment register)
     *  [esp+0x04]  fs              (segment register)
     *  [esp+0x08]  es              (segment register)
     *  [esp+0x0C]  ds              (segment register)
     *  [esp+0x10]  edi             (from pushal)
     *  [esp+0x14]  esi             (from pushal)
     *  [esp+0x18]  ebp             (from pushal)
     *  [esp+0x1C]  esp_dummy       (esp value before pushal, from pushal)
     *  [esp+0x20]  ebx             (from pushal)
     *  [esp+0x24]  edx             (from pushal)
     *  [esp+0x28]  ecx             (from pushal)
     *  [esp+0x2C]  eax             (from pushal)
     *  [esp+0x30]  vector          (pushed by ISR_* macros)
     *  [esp+0x34]  error_code      (pushed by ISR_* macros or CPU)
     *  [esp+0x38]  eip             (pushed by CPU on interrupt)
     *  [esp+0x3C]  cs              (pushed by CPU on interrupt)
     *  [esp+0x40]  eflags          (pushed by CPU on interrupt)
     *  [esp+0x44]  user_esp        (only if exception from user mode)
     *  [esp+0x48]  user_ss         (only if exception from user mode)
     */

    # Call C handler function: void exception_handler(regs_t *regs)
    # Pass pointer to saved register context on stack
    pushl %esp         # Argument: pointer to register context structure
    call exception_handler  # Call C function to handle exception
    addl $4, %esp      # Clean up parameter from stack (remove regs_t * argument)

    POP_TRAP_FRAME() /* Restore all registers and segment selectors from stack */

    # Clean up vector number and error code from stack before returning to interrupted code
    addl $8, %esp      # Remove vector (4 bytes) and error_code (4 bytes)

    # Return from interrupt: pops EIP, CS, EFLAGS (and optionally ESP, SS if from user mode)
    iret

.section .note.GNU-stack,"",@progbits
