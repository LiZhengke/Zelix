#ifndef PORTMACRO_H
#define PORTMACRO_H

#include <stdint.h>
#include "idt.h"

/* =========================================================
 * 1. 基础类型定义
 * ========================================================= */

typedef uint32_t StackType_t;
typedef int32_t  BaseType_t;
typedef uint32_t UBaseType_t;

typedef uint32_t TickType_t;
#define portMAX_DELAY    ( ( TickType_t ) 0xffffffffUL )

#define portSTACK_GROWTH         (-1)
#define portBYTE_ALIGNMENT    32
#define portTICK_PERIOD_MS       (1)
#define pvPortMalloc(size)    kmalloc(size)
#define vPortFree(ptr)       kfree(ptr)
/* =========================================================
 * 2. Critical section（x86 CLI/STI）
 * ========================================================= */
extern int task_started; /* Defined in main.c, indicates if the first task has started. */
static inline void portDISABLE_INTERRUPTS(void)
{
    __asm__ volatile ("cli");
}

static inline void portENABLE_INTERRUPTS(void)
{
    if(task_started) {
        __asm__ volatile ("sti");
    }
}

/* 简化版：单核可用 */
#define portENTER_CRITICAL()  portDISABLE_INTERRUPTS()
#define portEXIT_CRITICAL()   portENABLE_INTERRUPTS()

/* =========================================================
 * 3. Yield（触发调度）
 * ========================================================= */

static inline void portYIELD(void)
{
    __asm__ volatile ( "int %0" : : "i" ( portAPIC_YIELD_INT_VECTOR ) : "memory" );
}

#define portTASK_FUNCTION_PROTO( vFunction, pvParameters )    void vFunction( void * pvParameters )
#define portTASK_FUNCTION( vFunction, pvParameters )          void vFunction( void * pvParameters )

/* =========================================================
 * 4. Tick ISR hook
 * ========================================================= */

#define portSET_INTERRUPT_MASK_FROM_ISR()   0
#define portCLEAR_INTERRUPT_MASK_FROM_ISR(x)

/* =========================================================
 * 5. 上下文切换（最关键，但通常在 ASM）
 * ========================================================= */

/*
 * 你必须在 .S 文件实现：
 * - save context (pushad, push eip/eflags)
 * - restore context
 */

void vPortYieldFromISR(void);

#endif /* PORTMACRO_H */
