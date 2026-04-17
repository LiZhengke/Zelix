#include "adapter.h"
#include "ztask.h"

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
    TaskHandle_t tcb = xTaskGetCurrentTaskHandle();
    return sched_get_task(tcb);
}
