#pragma once

#include <stdint.h>
#include "ktask_internal.h"

struct mm_struct;
struct files_struct;

/* =========================
   task 状态
   ========================= */
enum task_state {
    TASK_RUNNING,
    TASK_ZOMBIE,
    TASK_DEAD,
};

/* =========================
   核心 task 结构
   ========================= */
struct task {
    /* FreeRTOS */
    tcb_t tcb;

    /* 进程资源 */
    struct mm_struct *mm;
    struct files_struct *files;

    /* 用户态 */
    struct user_context user_ctx;
    uint32_t *user_stack_top;

    /* 入口 */
    uint32_t *entry;

    /* 状态 */
    enum task_state state;
    int exit_code;

    /* 标志 */
    int started;

    /* 简单链表（可扩展） */
    struct task *next;
};

/* API */
struct task *task_create(const char *name, void *entry);
void task_destroy(struct task *t);

void do_exit(int code);
