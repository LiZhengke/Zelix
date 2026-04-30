#include "isr_support.h"
#include "asm_macros.h"

.extern pxCurrentTCB
.extern vTaskSwitchContext
.extern xTaskIncrementTick
.extern ulPortYieldPending
.extern save_user_ctx
.extern load_user_ctx
.extern current_task
.extern ulTopOfSystemStack
.extern ulInterruptNesting
.extern tss_set_esp0
.extern uSysCallDispatch
.extern task_started

.global vPortStartFirstTask
.global vPortYieldCall
.global vPortTimerHandler
.global vPortSysCall
.global gdt_flush
.global return_to_user


.text

.align 4
.func vUpdateTSS
vUpdateTSS:
    movl pxCurrentTCB, %eax
    test %eax, %eax
    jz .L_update_tss_exit

    movl (%eax), %ebp
    subl $TRAP_FRAME_SIZE, %ebp        /* adjust to point to top of stack for current task */
    push %ebp
    call tss_set_esp0
    addl $4, %esp   /* skip the parameter we just pushed for tss_set_esp0 */
.L_update_tss_exit:
    ret
.endfunc

.align 4
.func vPortStartFirstTask
vPortStartFirstTask:
    cli
    movl pxCurrentTCB, %eax
    movl (%eax), %esp        # load first task stack

    POP_TRAP_FRAME()

    movl %esp, %ebp        # set base pointer to top of stack for first task
    addl $12, %ebp        # adjust stack pointer to account for reserved task struct space
    pushl %ebp
    call tss_set_esp0  # 设置 TSS 的 esp0 为第一个任务的栈顶
    addl $4, %esp       # skip the parameter we just pushed for tss_set_esp0

    movl $1, task_started
    iret
.endfunc

.align 4
.func vPortYieldCall
vPortYieldCall:
    cli
    PUSH_TRAP_FRAME()

    movl task_started, %eax
    test %eax, %eax
    jz .L_yield_early_exit
    movl pxCurrentTCB, %eax
    test %eax, %eax
    jz .L_yield_early_exit
    movl %esp, (%eax)

    call current_task
    movl %eax, %ebx
    test %ebx, %ebx
    je .L_skip_save_ctx

    movl TASK_MM_OFFSET(%ebx), %eax
    test %eax, %eax
    je .L_skip_save_ctx

    pushl %esp
    pushl %eax
    call save_user_ctx
    addl $8, %esp

.L_skip_save_ctx:
    call vTaskSwitchContext

    movl pxCurrentTCB, %eax
    movl (%eax), %esp

    call current_task
    movl %eax, %ebx
    test %ebx, %ebx
    je .L_skip_load_ctx

    movl TASK_MM_OFFSET(%ebx), %eax
    test %eax, %eax
    je .L_skip_load_ctx

    movl %esp, %ebp
    addl $TRAP_FRAME_SIZE, %ebp        /* adjust to point to top of stack for current task */

    movl (%eax), %eax
    movl %eax, %cr3

    pushl %esp
    pushl %eax
    call load_user_ctx
    addl $8, %esp

.L_skip_load_ctx:
    pushl %ebp
    call tss_set_esp0
    addl $4, %esp   /* skip the parameter we just pushed for tss_set_esp0 */
.L_yield_early_exit:
    POP_TRAP_FRAME()
    iret
.endfunc

.align 4
.func vPortTimerHandler
vPortTimerHandler:
    PUSH_TRAP_FRAME()

    movl task_started, %eax
    test %eax, %eax
    jz .L_timer_early_exit

    movl pxCurrentTCB, %eax
    test %eax, %eax
    jz .L_timer_early_exit
    movl %esp, (%eax)

    call current_task
    movl %eax, %ebx
    test %ebx, %ebx
    je .L_timer_skip_save_ctx

    movl TASK_MM_OFFSET(%ebx), %eax
    test %eax, %eax
    je .L_timer_skip_save_ctx

    pushl %esp
    pushl %eax
    call save_user_ctx
    addl $8, %esp

.L_timer_skip_save_ctx:
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

    movl pxCurrentTCB, %eax
    movl (%eax), %esp

    call current_task
    movl %eax, %ebx
    test %ebx, %ebx
    je .L_timer_skip_load_ctx

    movl TASK_MM_OFFSET(%ebx), %eax
    test %eax, %eax
    je .L_timer_skip_load_ctx

    movl %esp, %ebp
    addl $TRAP_FRAME_SIZE, %ebp        /* adjust to point to top of stack for current task */

    movl (%eax), %eax
    movl %eax, %cr3

    pushl %esp
    pushl %eax
    call load_user_ctx
    addl $8, %esp

.L_timer_skip_load_ctx:
    pushl %ebp
    call tss_set_esp0
    addl $4, %esp   /* skip the parameter we just pushed for tss_set_esp0 */

    /* Acknowledge IRQ0 on master PIC so the next tick can be delivered. */
    movb    $0x20, %al
    outb    %al, $0x20

.L_timer_early_exit:
    POP_TRAP_FRAME()

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

    call    uSysCallDispatch       /* Call C function to handle syscall */
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

.align 4
.func return_to_user
return_to_user:
    /* 参数：tf 在栈上 */
    mov 4(%esp), %esp   /* 切到 tf 作为当前栈 */

    POP_TRAP_FRAME()             /* 恢复用户态寄存器*/

    /* iret：弹 eip, cs, eflags, esp, ss */
    iret
.endfunc

.section .note.GNU-stack,"",@progbits
