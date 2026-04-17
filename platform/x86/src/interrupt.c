#include <stdio.h>
#include "interrupt.h"
#include "io.h"

#define SYSTEM_STACK_SIZE 512
/* The stack used by interrupt handlers. */
static uint32_t ulSystemStack[ SYSTEM_STACK_SIZE ] __attribute__( ( used ) ) = { 0 };

/* Don't use the very top of the system stack so the return address
 * appears as 0 if the debugger tries to unwind the stack. */
volatile uint32_t ulTopOfSystemStack __attribute__( ( used ) );

/* If a yield is requested from an interrupt or from a critical section then
 * the yield is not performed immediately, and ulPortYieldPending is set to pdTRUE
 * instead to indicate the yield should be performed at the end of the interrupt
 * when the critical section is exited. */
volatile uint32_t ulPortYieldPending __attribute__( ( used ) ) = 0;

/* Counts the interrupt nesting depth.  Used to know when to switch to the
 * interrupt/system stack and when to save/restore a complete context. */
volatile uint32_t ulInterruptNesting __attribute__( ( used ) ) = 0;

/* Exception handler implementation */
void exception_handler(struct exc_regs *r) {
    // Print exception information
    printf("Exception Handler - Vector: %d, Error Code: %d\n", r->vector, r->err_code);
    printf("  Segments: CS=0x%x DS=0x%x ES=0x%x GS=0x%x\n", r->cs, r->ds, r->es, r->gs);
    printf("  Execution: EIP=0x%x EFLAGS=0x%x\n", r->eip, r->eflags);
    printf("  User Stack: ESP=0x%x SS=0x%x\n", r->user_esp, r->user_ss);

    // Only hardware interrupts (32-47) need EOI
    if (r->vector >= 32 && r->vector <= 47) {
        if (r->vector >= 40) {
            // If from slave PIC (IRQ8-15), send EOI to slave as well
            outb(0xA0, 0x20);
        }
        // Always send EOI to master PIC
        outb(0x20, 0x20);
    }
}

uint32_t get_cpu_cpl(){
    uint32_t cpl;
    __asm__ volatile (
        "mov %%cs, %0"
        : "=r" (cpl)
    );
    return cpl & 0x3; // CPL is in the lower 2 bits of CS
}
