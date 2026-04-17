// platform/x86/port.c

#include "FreeRTOS.h"
#include "task.h"

/* 外部汇编函数 */
extern void vPortStartFirstTask(void);
extern void vPortYieldCall(void);

/* =========================
   启动调度器
   ========================= */
BaseType_t xPortStartScheduler(void)
{
    vPortStartFirstTask();
    return 0;
}

/* =========================
   Yield（触发调度）
   ========================= */
void vPortYield(void)
{
    vPortYieldCall();
}

void vPortEndScheduler(void)
{
    for (;;) {
        __asm__ volatile ("cli; hlt");
    }
}

/* =========================
   初始化栈（关键点）
   ========================= */

StackType_t *pxPortInitialiseStack(
    StackType_t *pxTopOfStack,
    TaskFunction_t pxCode,
    void *pvParameters)
{
    /* 模拟中断返回栈结构 */

    *(--pxTopOfStack) = 0x01000000;         // EFLAGS (IF=1)
    *(--pxTopOfStack) = (StackType_t)pxCode; // EIP

    *(--pxTopOfStack) = 0xAAAAAAAA; // EAX
    *(--pxTopOfStack) = 0xBBBBBBBB; // ECX
    *(--pxTopOfStack) = 0xCCCCCCCC; // EDX
    *(--pxTopOfStack) = 0xDDDDDDDD; // EBX

    *(--pxTopOfStack) = (StackType_t)pvParameters; // ESP (fake)
    *(--pxTopOfStack) = 0xEEEEEEEE; // EBP
    *(--pxTopOfStack) = 0xFFFFFFFF; // ESI
    *(--pxTopOfStack) = 0x11111111; // EDI

    return pxTopOfStack;
}
