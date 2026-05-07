#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include "syscall.h"
#include "FreeRTOS.h"
#include "task.h"
#include "ktask.h"
#include "adapter.h"
#include "semphr.h"
#include "tss.h"
#include "interrupt.h"
#include "loader.h"
#include "task_context.h"
#include "io.h"

#define COM1_PORT 0x3F8
#define UART_LSR 5
#define UART_LSR_DR 0x01

// Forward declaration
extern int printf(const char *__restrict __format, ...);
extern int printf_va(const char *__restrict __format, va_list *__ap);
extern volatile uint32_t ulPortYieldPending;
typedef int (*syscall_t)(uint32_t, uint32_t,
                         uint32_t, uint32_t, uint32_t);
/* syscall functions interface */
static int sys_yield(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_write(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_delay(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_time_get(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_sem_pend(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_sem_post(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_putc(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_printf(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_panic(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_task_create(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_tick_count(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_get_task_name(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_exec(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_read(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);
static int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4);

const syscall_t syscall_table[SYS_MAX] = {
    sys_yield,
    sys_write,
    sys_delay,
    sys_exit,
    sys_time_get,
    sys_sem_pend,
    sys_sem_post,
    sys_putc,
    sys_printf,
    sys_panic,
    sys_task_create,
    sys_tick_count,
    sys_get_task_name,
    sys_exec,
    sys_read,
};


// int uSysCallDispatch(struct trap_frame *tf)
int uSysCallDispatch(void)
{
    uint32_t num, a0, a1, a2, a3, a4;

    asm volatile(
        "movl 52(%%ebp), %0\n"  // EAX saved by pushal
        "movl 40(%%ebp), %1\n"  // EBX
        "movl 48(%%ebp), %2\n"  // ECX
        "movl 44(%%ebp), %3\n"  // EDX
        "movl 28(%%ebp), %4\n"  // ESI
        "movl 24(%%ebp), %5\n"   // EDI
        : "=a"(num), "=b"(a0), "=c"(a1),
          "=d"(a2), "=S"(a3), "=D"(a4)
    );
/*
    num = tf->eax;
    a0 = tf->ebx;
    a1 = tf->ecx;
    a2 = tf->edx;
    a3 = tf->esi;
    a4 = tf->edi;
*/
    if (num >= SYS_MAX)
        return -OS_ERR_INVALID;

    return syscall_table[num](a0,a1,a2,a3,a4);
}

int os_err_to_errno(OS_ERR err)
{
    switch (err) {
    case OS_ERR_INVALID:
        return -EINVAL;
    case OS_ERR_PERM:
        return -EPERM;
    default:
        return -EINVAL;
    }
}

static int sys_yield(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    /* Defer context switch to interrupt epilogue instead of nesting a yield interrupt inside syscall handling. */
    ulPortYieldPending = 1U;
    return 0;
}

static int sys_write(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a3; (void)a4;
    int fd = a0;
    if (fd != 1 && fd != 2) // Only support stdout and stderr for now
        return -EINVAL;
    const char *str = (const char *)a1;
    if (str == NULL)
        return -EINVAL;
    int len = a2;
    if (len == 0)        return 0;
    if (len < 0)         return -EINVAL;
    for(int i = 0; i < len; i++) {
        putchar(str[i]);
        if (str[i] == '\n') // Convert newline to carriage return + newline
            putchar('\r');
    }
    return len;
}

static int sys_read(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a3; (void)a4;
    int fd = (int)a0;
    char *buf = (char *)a1;
    int len = (int)a2;
    int i;

    if (fd != 0)
        return -EINVAL;
    if (buf == NULL || len <= 0)
        return -EINVAL;

    for (i = 0; i < len; i++) {
        if ((inb(COM1_PORT + UART_LSR) & UART_LSR_DR) == 0U) {
            break;
        }
        buf[i] = (char)inb(COM1_PORT);
    }

    return i;
}

static int sys_delay(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    uint16_t ticks = a0;

    vTaskDelay(ticks);
    return 0;
}

static int sys_time_get(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    uint32_t *ticks = (uint32_t *)a0;
    *ticks = xTaskGetTickCount();
    return 0;
}

static int sys_sem_pend(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement semaphore pend using FreeRTOS APIs
    return -EINVAL;
}

static int sys_sem_post(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement semaphore post using FreeRTOS APIs
    return -EINVAL;
}

static int sys_panic(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    // TODO: Implement panic handler
    while(1);
    return 0;
}

static int sys_putc(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a1; (void)a2; (void)a3; (void)a4;
    char ch = a0;
    putchar((char)ch);
    return 0;
}

static int sys_printf(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a2; (void)a3; (void)a4;
    const char *fmt = (const char *)a0;
    va_list *args = (va_list *)a1;
    if (fmt == NULL || args == NULL)
        return -EINVAL;
    return printf_va(fmt, args);
}

static int sys_task_create(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    TaskFunction_t taskFunction = (TaskFunction_t)a0;
    const char *taskName = (const char *)a1;
    configSTACK_DEPTH_TYPE stackDepth = (configSTACK_DEPTH_TYPE)a2;
    UBaseType_t priority = (UBaseType_t)a3;
    void *pvParameters = (void *)a4;

    if (taskFunction == NULL){
        printf("sys_task_create: taskFunction is NULL\n");
        return -EINVAL;
    }

    if (priority >= configMAX_PRIORITIES) {
        printf("sys_task_create: invalid priority %u\n", priority);
        return -EINVAL;
    }

    if (stackDepth == 0)
        stackDepth = configMINIMAL_STACK_SIZE;

    /* Dynamically allocate TCB and stacks so multiple tasks can be created. */
    StaticTask_t *pxTCB = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
    StackType_t *pxKernelStack = (StackType_t *)pvPortMalloc(stackDepth * sizeof(StackType_t));
    StackType_t *pxUserStack = (StackType_t *)pvPortMalloc(stackDepth * sizeof(StackType_t));

    if (pxTCB == NULL || pxKernelStack == NULL || pxUserStack == NULL) {
        printf("Failed to allocate memory for task creation\n");
        printf("pxTCB=%p pxKernelStack=%p pxUserStack=%p\n", (void *)pxTCB, (void *)pxKernelStack, (void *)pxUserStack);
        if (pxTCB) vPortFree(pxTCB);
        if (pxKernelStack) vPortFree(pxKernelStack);
        if (pxUserStack) vPortFree(pxUserStack);
        return -EINVAL;
    }

    TaskHandle_t handle;
    BaseType_t ret = sched_task_create( taskFunction,
                                taskName,
                                stackDepth,
                                pvParameters,
                                priority,
                                &handle );

    if (ret != pdPASS) {
        printf("Failed to create task\n");
        vPortFree(pxTCB);
        vPortFree(pxKernelStack);
        vPortFree(pxUserStack);
        return -EINVAL;
    }

    printf("Task '%s' created successfully with priority %u\n", taskName, priority);
    return 0;
}

static int sys_tick_count(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a0; (void)a1; (void)a2; (void)a3; (void)a4;
    return (int)xTaskGetTickCount();
}

static int sys_get_task_name(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    (void)a2; (void)a3; (void)a4;
    char *buf = (char *)a0;
    uint32_t len = a1;

    if (buf == NULL || len == 0)
        return -EINVAL;

    const char *name = pcTaskGetName(NULL);
    uint32_t i;
    for (i = 0; i < len - 1 && name[i] != '\0'; i++)
        buf[i] = name[i];
    buf[i] = '\0';
    return 0;
}

static int do_exec(struct task *task,
            const char *path,
            const void *arg)
{
    mm_struct *new_mm, *old_mm;
    void *entry;

    if (task == NULL || path == NULL)
        return -EINVAL;

    task->arg = (void *)arg;
    task->state = TASK_RUNNING;
    task->type = TASK_TYPE_USER_PROCESS;

    task->entry_virt = USER_ENTRY_ADDRESS;
    task->user_stack_top = (uint32_t*)USER_STACK_ADDRESS;
    task->user_stack_size =
        USER_STACK_SIZE / sizeof(StackType_t);

    // 创建新地址空间
    new_mm = mm_create(task, path);
    if (new_mm == NULL)
        return -ENOMEM;

    // 加载 ELF
    entry = elf_load(path, new_mm->pgd, false);
    if (entry == NULL) {
        mm_destroy(new_mm);
        return -ENOEXEC;
    }

    task->entry = (task_entry_t)entry;
    task->entry_virt = (uint32_t)entry;
    task->started = 1;

    old_mm = task->mm;
    task->mm = new_mm;

    // setup trapframe
    arch_task_setup_frame_context(task);

    // 旧 mm 回收
    if (old_mm)
        mm_destroy(old_mm);

    return 0;
}

static int sys_exec(uint32_t a0,
                    uint32_t a1,
                    uint32_t a2,
                    uint32_t a3,
                    uint32_t a4)
{
    const char *path = (const char *)a0;
    const void *arg = (const void *)a1;

    struct task *current = current_task();

    int ret;

    (void)a2;
    (void)a3;
    (void)a4;

    ret = do_exec(current, path, arg);
    if (ret < 0)
        return ret;

    // 切换地址空间
    load_page_directory(current->mm->pgd_phys);

    // 进入用户态
    return_to_user(current->frame_ctx);

    __builtin_unreachable();
}

static void spawn_task_main(void *arg)
{
    (void)arg;

    for (;;) {
         TickType_t tickCount = sched_task_get_tick_count();
         if( tickCount % 100 == 0 )  /* Print every 100 ticks. */
         {
             printf( "spawn_task_main Tick: %lu cpl=%d\n", ( unsigned long ) tickCount, get_cpl() );
         }
         sched_delay(100);
     }
}
static int sys_spawn(uint32_t a0,
                     uint32_t a1,
                     uint32_t a2,
                     uint32_t a3,
                     uint32_t a4)
{
    const char *path = (const char *)a0;
    const void *arg  = (const void *)a1;

    struct task *task;
    int ret;

    (void)a2;
    (void)a3;
    (void)a4;

    // 创建新 task
    task = task_create(path,
                       spawn_task_main,
                       TASK_TYPE_USER_PROCESS,
                       USER_STACK_SIZE / sizeof(StackType_t),
                       NULL);

    if (!task)
        return -ENOMEM;

    // 对新 task 执行 exec
    ret = do_exec(task, path, arg);
    if (ret < 0) {
        task_destroy(task);
        return ret;
    }

    // 加入调度
    sched_task_wake_up(task);

    return task->pid;
}

#ifdef SYS_FORK_ENABLE
static int sys_fork(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    struct task *current = current_task();
    struct task *parent = current;

    struct task *child = alloc_task();  // 你已有或实现一个
    child->pid = next_pid++;

    /* 1️⃣ 复制地址空间（先用“深拷贝”，别一上来就 COW） */
    child->mm = mm_clone(parent->mm);

    /* 2️⃣ 复制 kernel stack（关键） */
    child->kstack_base = alloc_kstack();
    memcpy(child->kstack_base, parent->kstack_base, KSTACK_SIZE);

    /* 3️⃣ 修正 child 的 trap_frame 指针 */
    uint32_t offset = (uint32_t)parent->frame_ctx - (uint32_t)parent->kstack_base;
    child->frame_ctx = (struct trap_frame *)((uint32_t)child->kstack_base + offset);

    /* 4️⃣ fork 语义 */
    child->frame_ctx->eax = 0;                // 子进程返回 0
    parent->frame_ctx->eax = child->pid;      // 父进程返回 pid

    /* 5️⃣ 关系 */
    child->parent = parent;
    list_add(&child->sibling, &parent->children);

    /* 6️⃣ 状态 */
    child->state = TASK_READY;
    child->is_user = true;

    /* 7️⃣ 加入调度（通过 adapter → FreeRTOS） */
    sched_add_task(child);

    return child->pid;
}
#endif /* SYS_FORK_ENABLE */

static int sys_exit(uint32_t a0,uint32_t a1,uint32_t a2,uint32_t a3,uint32_t a4)
{
    int code = (int)a0;
    (void)a1; (void)a2; (void)a3; (void)a4;
    task_exit(code);
    // schedule();
}

/*--------------------------------------------------------------------- */
/* User-space syscall wrappers. These functions can be called by user tasks to
 * invoke system calls.
 *--------------------------------------------------------------------- */
int32_t uSysPutChar(char c)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_PUTC),   // eax: syscall number
          "b"(c)           // ebx: argument
        : "memory"
    );
    return ret;
}

int32_t uSysDelay(uint16_t ticks)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_DELAY),   // eax: syscall number
          "b"(ticks)        // ebx: argument
        : "memory"
    );
    return ret;
}

int32_t uSysTaskCreate(void (*taskFunction)(void *), const char *taskName,
                       uint16_t stackDepth, uint32_t priority,
                       void *pvParameters)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_TASK_CREATE),
          "b"(taskFunction),
          "c"(taskName),
          "d"((uint32_t)stackDepth),
          "S"(priority),
          "D"(pvParameters)
        : "memory"
    );
    return ret;
}

int32_t uSysGetTickCount(void)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_TICK_COUNT)
        : "memory"
    );
    return ret;
}

int32_t uSysPrintf(const char *fmt, ...)
{
    int32_t ret;
    va_list args;
    va_start(args, fmt);

    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_PRINTF),    // eax: syscall number
          "b"(fmt),           // ebx: format string pointer
          "c"(&args)          // ecx: pointer to va_list
        : "memory"
    );

    va_end(args);
    return ret;
}

int32_t uSysGetTaskName(char *buf, uint32_t len)
{
    int32_t ret;
    asm volatile (
        "int $" STR(SYSINT)
        : "=a"(ret)
        : "a"(SYS_GET_TASK_NAME),
          "b"(buf),
          "c"(len)
        : "memory"
    );
    return ret;
}
