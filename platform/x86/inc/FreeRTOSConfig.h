#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#define configUSE_PREEMPTION            1
#define configUSE_IDLE_HOOK             1
#define configUSE_TICK_HOOK             1

#define configCPU_CLOCK_HZ    ( ( unsigned long ) 20000000 )
#define configTICK_RATE_HZ              100

#define configMAX_PRIORITIES            5
#define configMINIMAL_STACK_SIZE        256

#define configTOTAL_HEAP_SIZE           (1024 * 1024)
#define configISR_STACK_SIZE            512

#define configUSE_16_BIT_TICKS          0

#define configUSE_MUTEXES               1
#define configUSE_COUNTING_SEMAPHORES   1

#define configUSE_APPLICATION_TASK_TAG  1

#define INCLUDE_vTaskDelay              1
#define INCLUDE_vTaskDelete             1

#endif /* FREERTOS_CONFIG_H */
