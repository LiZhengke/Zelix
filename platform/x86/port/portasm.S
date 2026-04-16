.global vPortStartFirstTask
.global vPortYieldCall
.global vPortTimerHandler
.global vPortSwitchContext
.extern pxCurrentTCB

vPortStartFirstTask:
    movl pxCurrentTCB, %eax
    movl (%eax), %esp        # load first task stack

    popl %edi
    popl %esi
    popl %ebp
    add $4, %esp            # skip ESP placeholder
    popl %ebx
    popl %edx
    popl %ecx
    popl %eax

    iret

vPortYieldCall:
    cli
    pushl %eax
    pushl %ecx
    pushl %edx

    call vTaskSwitchContext

    popl %edx
    popl %ecx
    popl %eax
    sti
    ret

vPortTimerHandler:
    cli

    pushal

    call xTaskIncrementTick
    call vTaskSwitchContext

    popal

    sti
    iret

vPortSwitchContext:
    # 保存当前 task context
    pushal

    movl pxCurrentTCB, %eax
    movl %esp, (%eax)

    # 切换 task（C 里做）
    call vTaskSwitchContext

    # 恢复新 task
    movl pxCurrentTCB, %eax
    movl (%eax), %esp

    popal
    ret
