#include <string.h>
#include "ktask.h"
#include "adapter.h"
#include "arch.h"
#include "stdio.h"
#include "task_context.h"

#define STACK_SIZE 4096
#define USER_STACK_SIZE 4096

static struct task *task_list = NULL;

static struct task *alloc_task(void)
{
    struct task *t = sched_task_alloc(sizeof(*t));
    if (!t) return NULL;

    memset(t, 0, sizeof(*t));
    return t;
}

static void *alloc_user_stack(struct task *t)
{
    void *stack = sched_task_alloc(USER_STACK_SIZE);
    return (uint8_t *)stack + USER_STACK_SIZE;
}


void kernel_task_entry(void *arg)
{
    struct task *t = (struct task *)arg;

    sched_bind_task(sched_current_handle(), t);
    t->started = 1;

    if (t->entry != NULL) {
        arch_task_setup_frame_context(t);
        return_to_user(t->frame_ctx);
    }

    for (;;) {
        TickType_t tickCount = sched_task_get_tick_count();
        if( tickCount % 100 == 0 )  /* Print every 100 ticks. */
        {
            printf( "kernel_task_entry Tick: %lu cpl=%d\n", ( unsigned long ) tickCount, get_cpl() );
        }
        sched_delay(100);
    }
}

struct task *task_create(const char *name, task_entry_t entry, enum task_type type, void *arg)
{
    struct task *t = alloc_task();
    BaseType_t status;

    if ((t == NULL) || (entry == NULL)) {
        if (t != NULL) {
            sched_task_free(t);
        }
        return NULL;
    }

    t->entry = entry;
    t->arg = arg;
    t->state = TASK_RUNNING;
    t->type = type;
    // t->mm = mm_create(t);
    t->user_stack_top = alloc_user_stack(t);

    status = sched_task_create(
        kernel_task_entry,
        name,
        STACK_SIZE,
        t,
        1,
        &t->tcb
    );

    if (status != pdPASS) {
        sched_task_free(t);
        return NULL;
    }

    t->next = task_list;
    task_list = t;

    return t;
}

struct task *task_system_start(const char *name, task_entry_t entry, enum task_type type, void *arg)
{
    struct task *task = task_create(name, entry, type, arg);

    if (task == NULL) {
        return NULL;
    }

    task_scheduler_start();
    return task;
}

void task_scheduler_start(void)
{
    sched_start();
}

void task_destroy(struct task *t)
{
    (void)t;
}

void do_exit(int code)
{
    struct task *t = current_task();

    t->exit_code = code;
    t->state = TASK_ZOMBIE;

    /* 释放资源（简化版） */
    if (t->mm)
        destroy_mm(t->mm);

    if (t->user_stack_top)
        sched_task_free((void *)((uint32_t)t->user_stack_top - USER_STACK_SIZE));

    /* 删除 FreeRTOS task */
    sched_task_delete(NULL);

    for (;;);
}



void user_exit_trampoline(void)
{
    do_exit(0);
}
