#ifndef _SCHED_ADAPTER_H
#define _SCHED_ADAPTER_H

#include "FreeRTOS.h"
#include "task.h"

struct task;

/* 绑定 */
void sched_bind_task(TaskHandle_t tcb, struct task *task);

/* 获取 */
struct task *sched_get_task(TaskHandle_t tcb);

/* 当前任务 */
struct task *current_task(void);

#endif /* _SCHED_ADAPTER_H */
