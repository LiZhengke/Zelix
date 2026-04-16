#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             0
#define configUSE_TICK_HOOK             0

#define configCPU_CLOCK_HZ              100000000
#define configTICK_RATE_HZ              1000

#define configMAX_PRIORITIES            5
#define configMINIMAL_STACK_SIZE        256

#define configTOTAL_HEAP_SIZE           (1024 * 1024)

#define configUSE_16_BIT_TICKS          0

#define configUSE_MUTEXES               1
#define configUSE_COUNTING_SEMAPHORES   1

#define configUSE_APPLICATION_TASK_TAG  1

#endif /* FREERTOS_CONFIG_H */
