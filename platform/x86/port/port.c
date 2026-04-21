// platform/x86/port.c

#include <stdio.h>

#include "arch.h"
#include "FreeRTOS.h"
#include "task.h"
#include "gdt.h"

#define portINITIAL_EFLAGS               ( 0x200UL )

#ifndef configENABLE_PRINT_ESP
    #define configENABLE_PRINT_ESP 0
#endif
/* The expected size of each entry in the IDT.  Used to check structure packing
 * is set correctly. */
#define portEXPECTED_IDT_ENTRY_SIZE      8
/* The value used as a magic number to fill the stack of a task when it is
 * created.  This is just useful for debugging and can be set to any value. */
#define portSTACK_WORD                  0xA5A5A5A5UL

#define portHOOK_PRINT_EVERY 100U

#if ( configENABLE_PRINT_ESP == 1 )
    extern unsigned long get_esp( void );
#endif

/* The stack used by interrupt handlers. */
static uint32_t ulSystemStack[ configISR_STACK_SIZE ] __attribute__( ( used ) ) = { 0 };

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
/* 外部汇编函数 */
extern void vPortStartFirstTask(void);
extern void vPortYieldCall(void);

volatile uint32_t ulCriticalNesting = 9999UL;

/* =========================
   启动调度器
   ========================= */
BaseType_t xPortStartScheduler(void)
{
    BaseType_t xWord;

    /* Some versions of GCC require the -mno-ms-bitfields command line option
     * for packing to work. */
    configASSERT( sizeof( struct IDTEntry ) == portEXPECTED_IDT_ENTRY_SIZE );

    (void) xWord;
    ulTopOfSystemStack =
    (uint32_t)&(ulSystemStack[ configISR_STACK_SIZE - 5 ]);

    /* Fill part of the system stack with a known value to help detect stack
     * overflow.  A few zeros are left so GDB doesn't get confused unwinding
     * the stack. */
    for( xWord = 0; xWord < configISR_STACK_SIZE - 20; xWord++ )
    {
        ulSystemStack[ xWord ] = portSTACK_WORD;
    }

      /* Make sure the stack used by interrupts is aligned. */
    ulTopOfSystemStack &= ~portBYTE_ALIGNMENT_MASK;
    ulCriticalNesting = 0;

    vPortStartFirstTask();
    return 0;
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

    *(--pxTopOfStack) = (StackType_t)0;
    *(--pxTopOfStack) = (StackType_t)0;

    *(--pxTopOfStack) = (StackType_t)pvParameters;         // 用户态参数
    *(--pxTopOfStack) = (StackType_t)0;         // 用户态返回地址（不存在，填0）


    *(--pxTopOfStack) = portINITIAL_EFLAGS;         // EFLAGS (IF=1)
    *(--pxTopOfStack) = KERNEL_CS;         // CS (用户态代码段选择子)
    *(--pxTopOfStack) = (StackType_t)pxCode; // EIP

    *(--pxTopOfStack) = 0xAAAAAAAA; // EAX
    *(--pxTopOfStack) = 0xBBBBBBBB; // ECX
    *(--pxTopOfStack) = 0xCCCCCCCC; // EDX
    *(--pxTopOfStack) = 0xDDDDDDDD; // EBX

    *(--pxTopOfStack) = (StackType_t)(pxTopOfStack + 4); // ESP (fake) - must point to EBX location for popal to work
    *(--pxTopOfStack) = 0xEEEEEEEE; // EBP
    *(--pxTopOfStack) = 0xFFFFFFFF; // ESI
    *(--pxTopOfStack) = 0x11111111; // EDI
    *(--pxTopOfStack) = KERNEL_DS;  // DS
    *(--pxTopOfStack) = KERNEL_DS;  // ES
    *(--pxTopOfStack) = KERNEL_DS;  // FS
    *(--pxTopOfStack) = KERNEL_DS;  // GS

    return pxTopOfStack;
}

#if ( configCHECK_FOR_STACK_OVERFLOW > 0 )

    void vApplicationStackOverflowHook( TaskHandle_t xTask,
                                        char * pcTaskName )
    {
        /* Check pcTaskName for the name of the offending task,
         * or pxCurrentTCB if pcTaskName has itself been corrupted. */
        ( void ) xTask;
        ( void ) pcTaskName;
    }

#endif /* #if ( configCHECK_FOR_STACK_OVERFLOW > 0 ) */
/*-----------------------------------------------------------*/
#if  ( configUSE_TICK_HOOK != 0 )
    void vApplicationTickHook( void )
    {
        /* This function will be called by each tick interrupt if
         * configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h. */
        TickType_t tickCount = xTaskGetTickCount();
        if( ( tickCount % portHOOK_PRINT_EVERY ) == 0U ) {
            printf( "Tick Hook: %lu cpl=%d\n", ( unsigned long ) tickCount, get_cpl() );
#if configENABLE_PRINT_ESP == 1
            printf( "Tick Hook: esp=%p\n", ( void * ) get_esp() );
#endif /* configENABLE_PRINT_ESP */
        }
    }
#endif/* ( configUSE_TICK_HOOK != 0 ) */
/*-----------------------------------------------------------*/

#if ( configUSE_IDLE_HOOK == 1 )
    void vApplicationIdleHook( void )
    {
        /* This function will be called by the idle task if
         * configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h. */
        static TickType_t last = 0;
        TickType_t now = xTaskGetTickCount();
#if configENABLE_PRINT_ESP == 1
        unsigned long esp;
#endif /* configENABLE_PRINT_ESP == 1 */
        if( now >= ( last + portHOOK_PRINT_EVERY ) ) {
            printf( "Idle: tick=%lu cpl=%d\n", ( unsigned long ) now, get_cpl() );
#if configENABLE_PRINT_ESP == 1
            esp = get_esp();
            printf( "Idle: esp=%p\n", ( void * ) esp );
#endif /* configENABLE_PRINT_ESP */
            last = now;
        }
    }
#endif /* configUSE_IDLE_HOOK == 1 */
