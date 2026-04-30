#pragma once

#include <stdint.h>
#include "ktask_internal.h"
#include "mm.h"

#define USER_STACK_SIZE (16*4096)
#define KERNEL_STACK_SIZE (26*4096)

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
    TASK_TYPE_USER_PROCESS,
    TASK_TYPE_KERNEL_THREAD,
};
/* =========================
   核心 task 结构
   ========================= */
struct task {
    /* FreeRTOS */
    tcb_t tcb;

    /* 进程资源 */
    mm_struct *mm;
    mm_struct *active_mm; /* For threads sharing the same address space */
    struct files_struct *files;

    struct trap_frame* frame_ctx;
    uint32_t *kernel_stack_top;
    size_t kernel_stack_size;

    uint32_t *user_stack_top;
    size_t user_stack_size;

    /* 入口 */
    task_entry_t entry;
    uint32_t entry_virt; /* 虚拟地址形式的入口，供用户进程使用 */
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
struct task *task_create(const char *name, task_entry_t entry, enum task_type type, size_t user_stack_size, void *arg);
struct task *task_system_start(const char *name, task_entry_t entry, enum task_type type, size_t user_stack_size, void *arg);
void task_scheduler_start(void);
void task_destroy(struct task *t);

int is_user_task(struct task *t);
int is_kernel_thread(struct task *t);

void do_exit(int code);
