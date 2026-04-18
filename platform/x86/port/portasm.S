#include "isr_support.h"
#include "asm_macros.h"

.extern pxCurrentTCB
.extern vTaskSwitchContext
.extern xTaskIncrementTick
.extern ulPortYieldPending

.global vPortStartFirstTask
.global vPortYieldCall
.global vPortTimerHandler
.global vPortSysCall
.global gdt_flush

.text

.align 4
.func vPortStartFirstTask
vPortStartFirstTask:
    movl pxCurrentTCB, %eax
    movl (%eax), %esp        # load first task stack

    popal
    iret
.endfunc

.align 4
.func vPortYieldCall
vPortYieldCall:
    cli
    pushal

    movl pxCurrentTCB, %eax
    movl %esp, (%eax)

    call vTaskSwitchContext

    movl    pxCurrentTCB, %eax
    movl    (%eax), %esp

    popal
    iret
.endfunc

.align 4
.func vPortTimerHandler
vPortTimerHandler:
    pushal

    movl    pxCurrentTCB, %eax
    movl    %esp, (%eax)

    movl    ulTopOfSystemStack, %esp
    movl    %esp, %ebp

    /* Increment nesting count. */
    addl    $1, ulInterruptNesting

    call xTaskIncrementTick
    sti

    /* Is a switch to another task required? */
    test    %eax, %eax
    je      _skip_context_switch
    cli
    call vTaskSwitchContext

_skip_context_switch:
    cli

    /* Decrement the variable used to determine if a switch to a system stack is necessary. */
    subl    $1, ulInterruptNesting

    movl    pxCurrentTCB, %eax
    movl    (%eax), %esp

    /* Acknowledge IRQ0 on master PIC so the next tick can be delivered. */
    movb    $0x20, %al
    outb    %al, $0x20

    popal

    sti
    iret
.endfunc

.align 4
.func vPortSysCall
vPortSysCall:
    cli
    pushal
    PUSH_SEGS()

        /*
        * Stack layout at this point (ESP → increases downward as we push):
        *   +-----------------+
        *   | GS              | ← ESP + 0 (top of stack after pushal)
        *   +-----------------+
        *   | FS              | ← ESP + 4
        *   +-----------------+
        *   | ES              | ← ESP + 8
        *   +-----------------+
        *   | DS              | ← ESP + 12
        *   +-----------------+
        *   | EDI             | ← ESP + 16
        *   +-----------------+
        *   | ESI             | ← ESP + 20
        *   +-----------------+
        *   | EBP             | ← ESP + 24
        *   +-----------------+
        *   | ESP (original)  | ← ESP + 28
        *   +-----------------+
        *   | EBX             | ← ESP + 32
        *   +-----------------+
        *   | EDX             | ← ESP + 36
        *   +-----------------+
        *   | ECX             | ← ESP + 40
        *   +-----------------+
        *   | EAX             | ← ESP + 44
        *   +-----------------+ (ESP points here)
        */

    mov     $0x10, %ax              /* KERNEL_DS 0x10 */
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    /* pushl %esp */                /* Parameter: regs_t * */

    /*!!!! TODO: we could optimize this by having the C handler read the syscall number
     directly from the saved EAX slot on the stack instead of pushing it as a parameter. */

    /*call    uSysCallDispatch */    /* Call C function to handle syscall */
    /* addl $4, %esp */

    /* Store the syscall return value (in EAX) into the saved-EAX slot
        * on the stack so that popal restores it instead of the original
        * EAX (which held the syscall number). */
    movl    %eax, 44(%esp)

    POP_SEGS()

        /*
        * Stack layout at this point (ESP → increases downward as we push):
        *   +-----------------+
        *   | GS              | ← ESP + 32 (top of stack after pushal)
        *   +-----------------+
        *   | FS              | ← ESP + 28
        *   +-----------------+
        *   | ES              | ← ESP + 24
        *   +-----------------+
        *   | DS              | ← ESP + 20
        *   +-----------------+
        *   | EDI             | ← ESP + 16
        *   +-----------------+
        *   | ESI             | ← ESP + 12
        *   +-----------------+
        *   | EBP             | ← ESP + 8
        *   +-----------------+
        *   | ESP (original)  | ← ESP + 4
        *   +-----------------+
        *   | EBX             | ← ESP (current value after pushal)
        *   +-----------------+
        *   | EDX             |
        *   +-----------------+
        *   | EDX             | ← ESP + 4
        *   | ECX             |
        *   +-----------------+
        *   | EAX             |
        *   +-----------------+ (ESP points here)
        */
    popal

    sti
    iret
.endfunc

/* GDT flush function - loads new GDT and reloads segment registers */
.align 4
.func gdt_flush
gdt_flush:
    movl 4(%esp), %eax      /* Get the pointer to the GDT descriptor */
    lgdt (%eax)             /* Load the new GDT */

    /* Reload all segment registers */
    movw $0x10, %ax         /* 0x10 is the offset in the GDT to our data segment */
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Far jump to reload CS with our new code segment (0x08) */
    ljmp $0x08, $flush_cs
flush_cs:
    ret
.endfunc

.section .note.GNU-stack,"",@progbits
