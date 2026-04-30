#include "adapter.h"
#include "heap_alloc.h"

void *sched_task_alloc(size_t size)
{
    size = (size + 7) & ~7;   // 8-byte 对齐
    return pvPortMalloc(size);
}

void sched_task_free(void *ptr)
{
    vPortFree(ptr);
}

void sched_task_delete(TaskHandle_t handle)
{
    vTaskDelete(handle);
}

BaseType_t sched_task_create(TaskFunction_t entry,
                             const char *name,
                             uint16_t stack_size,
                             void *arg,
                             UBaseType_t priority,
                             TaskHandle_t *handle)
{
    return xTaskCreate(entry, name, stack_size, arg, priority, handle);
}

void sched_start(void)
{
    vTaskStartScheduler();
}

void sched_delay(TickType_t ticks)
{
    vTaskDelay(ticks);
}

TickType_t sched_task_get_tick_count(void)
{
    return xTaskGetTickCount();
}

TaskHandle_t sched_current_handle(void)
{
    return xTaskGetCurrentTaskHandle();
}

void sched_bind_task(TaskHandle_t tcb, struct task *task)
{
    vTaskSetApplicationTaskTag(tcb, (TaskHookFunction_t)task);
}

struct task *sched_get_task(TaskHandle_t tcb)
{
    return (struct task *)xTaskGetApplicationTaskTag(tcb);
}

struct task *current_task(void)
{
    TaskHandle_t tcb = sched_current_handle();
    return sched_get_task(tcb);
}
