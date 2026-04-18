#include <string.h>
#include "ktask.h"
#include "adapter.h"
#include "arch.h"
#include "stdio.h"

#define STACK_SIZE 4096

static struct task *task_list = NULL;

static struct task *alloc_task(void)
{
    struct task *t = sched_task_alloc(sizeof(*t));
    if (!t) return NULL;

    memset(t, 0, sizeof(*t));
    return t;
}

void kernel_task_entry(void *arg)
{
    struct task *t = (struct task *)arg;

    sched_bind_task(sched_current_handle(), t);
    t->started = 1;

    if (t->entry != NULL) {
        t->entry(t->arg);
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

struct task *task_create(const char *name, task_entry_t entry, void *arg)
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

struct task *task_system_start(const char *name, task_entry_t entry, void *arg)
{
    struct task *task = task_create(name, entry, arg);

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
    (void)code;

    for (;;) {
    }
}
