#include <string.h>
#include "ktask.h"
#include "adapter.h"
#include "arch.h"
#include "stdio.h"
#include "task_context.h"

#define USER_STACK_ADDRESS 0xBFFFF000
#define USER_ENTRY_ADDRESS 0x08048000
int task_started = 0;

int is_user_task(struct task *t)
{
    return t && t->mm != NULL;
}

int is_kernel_thread(struct task *t)
{
    return t && t->mm == NULL;
}

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
    void *stack = sched_task_alloc(t->user_stack_size / sizeof(StackType_t));
    return (uint8_t *)stack + t->user_stack_size / sizeof(StackType_t);
}

static void kernel_thread_entry(void *arg)
{
    struct task *t = arg;
    t->entry(t->arg);
    do_exit(0);
}


static void kernel_task_entry(void *arg)
{
    struct task *t = (struct task *)arg;

    // taskENTER_CRITICAL();
    load_page_directory(t->mm->pgd_phys);
     /* Update TSS esp0 to the new kernel stack. */
     /* Note: For user processes, we also need to update TSS esp0 to point to the new kernel stack. For kernel threads, they already have their own kernel stack, so we just need to set esp0 to the top of that stack. */
    // if (is_user_task(t)) {
    //     tss_set_esp0((uint32_t)alloc_user_stack(t));
    // sched_bind_task(sched_current_handle(), t);
    t->started = 1;
    if (t->entry != NULL) {
        arch_task_setup_frame_context(t);
    }
    // taskEXIT_CRITICAL();

    if (t->entry != NULL) {
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

struct task *task_create(const char *name, task_entry_t entry, enum task_type type, size_t user_stack_size, void *arg)
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
    if(type == TASK_TYPE_USER_PROCESS) {
        t->entry_virt = USER_ENTRY_ADDRESS; /* For user processes, we use a fixed virtual address for the entry point. */
        t->user_stack_top = (uint32_t*)USER_STACK_ADDRESS;
        t->user_stack_size = user_stack_size;
        t->mm = mm_create(t, name);
    } else {
        t->user_stack_top = NULL;
        t->mm = NULL;
        t->frame_ctx = NULL;
    }

    status = sched_task_create(
        type == TASK_TYPE_USER_PROCESS ? kernel_task_entry : kernel_thread_entry,
        name,
        KERNEL_STACK_SIZE / sizeof(StackType_t),
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
    sched_bind_task(t->tcb, t);
    return t;
}

struct task *task_system_start(const char *name, task_entry_t entry, enum task_type type, size_t user_stack_size, void *arg)
{
    struct task *task = task_create(name, entry, type, user_stack_size, arg);

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
        sched_task_free((void *)((uint32_t)t->user_stack_top - t->user_stack_size));

        // wakeup_parent(t);
    /* 删除 FreeRTOS task */
    sched_task_delete(NULL);
}



void user_exit_trampoline(void)
{
    do_exit(0);
}
