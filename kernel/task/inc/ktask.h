#pragma once

#include <stdint.h>
#include "ktask_internal.h"
#include "mm.h"

struct files_struct;
typedef void (*task_entry_t)(void *arg);

/* =========================
   task 状态
   ========================= */
enum task_state {
    TASK_RUNNING,
    TASK_ZOMBIE,
    TASK_DEAD,
};

enum task_type {
    TASK_TYPE_PROCESS,
    TASK_TYPE_THREAD,
};
/* =========================
   核心 task 结构
   ========================= */
struct task {
    /* FreeRTOS */
    tcb_t tcb;

    /* 进程资源 */
    mm_struct *mm;
    struct files_struct *files;

    /* 用户态 */
    struct trap_frame* frame_ctx;
    uint32_t *user_stack_top;

    /* 入口 */
    task_entry_t entry;
    void *arg;

    /* 状态 */
    enum task_state state;
    enum task_type type;

    int exit_code;

    /* 标志 */
    int started;

    /* 简单链表（可扩展） */
    struct task *next;
};

/* API */
struct task *task_create(const char *name, task_entry_t entry, enum task_type type, void *arg);
struct task *task_system_start(const char *name, task_entry_t entry, enum task_type type, void *arg);
void task_scheduler_start(void);
void task_destroy(struct task *t);
void kernel_task_entry(void *arg);

void do_exit(int code);
