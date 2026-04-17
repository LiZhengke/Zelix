#include "task.h"
#include "mm/mm.h"
#include "sched/adapter.h"

#include <string.h>
#include <stdlib.h>   // 暂时可用，后面替换

#define STACK_SIZE 4096
#define USER_STACK_SIZE 4096

#define USER_CS 0x1B
#define USER_DS 0x23

static struct task *task_list = NULL;

static struct task *alloc_task(void)
{
    struct task *t = malloc(sizeof(*t));
    if (!t) return NULL;

    memset(t, 0, sizeof(*t));
    return t;
}

static void *alloc_user_stack(struct task *t)
{
    void *stack = malloc(USER_STACK_SIZE);
    return (uint8_t *)stack + USER_STACK_SIZE;
}

extern void user_exit_trampoline(void);

static void setup_user_context(struct task *t)
{
    uint32_t *sp = (uint32_t *)t->user_stack_top;

    /* main return → exit */
    *(--sp) = (uint32_t)user_exit_trampoline;

    t->user_ctx.eip = (uint32_t)t->entry;
    t->user_ctx.esp = (uint32_t)sp;
    t->user_ctx.eflags = 0x202;

    t->user_ctx.eax = 0;
    t->user_ctx.ebx = 0;
    t->user_ctx.ecx = 0;
    t->user_ctx.edx = 0;
}

extern void enter_user_mode(struct user_context *ctx);
extern void restore_user_and_iret(struct user_context *ctx);
extern void switch_to_mm(struct mm_struct *mm);

void kernel_task_entry(void *arg)
{
    struct task *t = (struct task *)arg;

    /* 设置 current */
    sched_bind_task(xTaskGetCurrentTaskHandle(), t);

    switch_to_mm(t->mm);

    if (!t->started) {
        t->started = 1;

        setup_user_context(t);
        enter_user_mode(&t->user_ctx);
    }

    restore_user_and_iret(&t->user_ctx);

    for (;;);
}

struct task *task_create(const char *name, void *entry)
{
    struct task *t = alloc_task();
    if (!t) return NULL;

    t->entry = entry;
    t->state = TASK_RUNNING;

    /* mm（最开始可以共享 kernel mm） */
    t->mm = create_kernel_mm();

    /* 用户栈 */
    t->user_stack_top = alloc_user_stack(t);

    /* 创建 FreeRTOS task */
    xTaskCreate(
        kernel_task_entry,
        name,
        STACK_SIZE,
        t,
        1,
        &t->tcb
    );

    /* ⭐ 绑定 */
    sched_bind_task(t->tcb, t);

    /* 加入链表 */
    t->next = task_list;
    task_list = t;

    return t;
}
