#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION            1
#define configUSE_TIMERS                (configMAX_PRIORITIES-1)
#define configTIMER_TASK_PRIORITY       2
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0

#define configCPU_CLOCK_HZ    ( ( unsigned long ) 20000000 )
#define configTICK_RATE_HZ              100

#define configMAX_PRIORITIES            32
#define configMINIMAL_STACK_SIZE        256

// #define configTOTAL_HEAP_SIZE       ((size_t)(&__heap_end - &__heap_start)) /* 64KB heap size for FreeRTOS. */
#define configTOTAL_HEAP_SIZE       ((size_t)(256 * 1024)) /* 256KB heap size for FreeRTOS. */

/* Set configAPPLICATION_ALLOCATED_HEAP to 1 to have the application allocate
 * the array used as the FreeRTOS heap.  Set to 0 to have the linker allocate
 * the array used as the FreeRTOS heap.  Defaults to 0 if left undefined. */
#define configAPPLICATION_ALLOCATED_HEAP             1
#define configISR_STACK_SIZE            512

#define configUSE_16_BIT_TICKS          0

#define configUSE_MUTEXES               1
#define configUSE_COUNTING_SEMAPHORES   1

#define configUSE_APPLICATION_TASK_TAG  1

#define INCLUDE_vTaskDelay              1
#define INCLUDE_vTaskDelete             1
#define configUSE_I8259                 1 /* Enable 8259 PIC support for timer interrupts. */

#define configASSERT( x )         \
    if( ( x ) == 0 )              \
    {                             \
        taskDISABLE_INTERRUPTS(); \
        for( ; ; )                \
        ;                         \
    }

#define STATIC_ASSERT( x )    \
    typedef char static_assertion_failed[ ( x ) ? 1 : -1 ]
#endif /* FREERTOS_CONFIG_H */
