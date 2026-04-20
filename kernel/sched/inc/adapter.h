#ifndef _SCHED_ADAPTER_H
#define _SCHED_ADAPTER_H

#include "FreeRTOS.h"
#include "task.h"
#include "ktask.h"

void *sched_task_alloc(size_t size);
void sched_task_free(void *ptr);
void sched_task_delete(TaskHandle_t handle);
BaseType_t sched_task_create(TaskFunction_t entry,
                             const char *name,
                             uint16_t stack_size,
                             void *arg,
                             UBaseType_t priority,
                             TaskHandle_t *handle);
void sched_start(void);
void sched_delay(TickType_t ticks);
TickType_t sched_task_get_tick_count(void);
TaskHandle_t sched_current_handle(void);

/* 绑定 */
void sched_bind_task(TaskHandle_t tcb, struct task *task);

/* 获取 */
struct task *sched_get_task(TaskHandle_t tcb);

/* 当前任务 */
struct task *current_task(void);

#endif /* _SCHED_ADAPTER_H */
