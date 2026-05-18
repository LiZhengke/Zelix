#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "ktask_internal.h"
#include "mm.h"
#include "zelix/zlist.h"
#include "zelix/zwait.h"

#define USER_STACK_SIZE (16*4096)
#define KERNEL_STACK_SIZE (26*4096)

#define USER_STACK_ADDRESS 0xBFFFF000
#define USER_ENTRY_ADDRESS 0x08048000

struct files_struct;
typedef void (*task_entry_t)(void *arg);

/* =========================
   task 状态
   ========================= */
enum task_state {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
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

    int pid;

    // --- 固有属性：保留直接嵌入 ---
    struct list_head global_node;   // 全系统任务表
    struct list_head sched_node;    // 调度链表（仅用于 run_queue 或 zombie_queue）

     /* 父子关系 */
    struct task *parent;
    struct list_head sibling_node; // 在父进程的 children_head 链表中的节点
    struct list_head children_head;

    bool is_user;

    /*
     * 当前阻塞在哪个 wait queue
     */
    wait_queue_t *wait_queue;

    /*
     * 用于插入 wait queue
     */
    wait_node_t wait_node;

     /* waitpid 专用 */
    wait_queue_t child_exit_wq;
};

/* API */
struct task *task_create(const char *name, task_entry_t entry, enum task_type type, size_t user_stack_size, struct task *parent, void *arg);
struct task *task_system_start(const char *name, task_entry_t entry, enum task_type type, size_t user_stack_size, void *arg);
void task_scheduler_start(void);
void task_destroy(struct task *t);

int is_user_task(struct task *t);
int is_kernel_thread(struct task *t);
struct task *alloc_task(void);
void task_exit(int code);

