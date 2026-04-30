#include <stdio.h>
#include <stdbool.h>
#include "ktask.h"
#include "adapter.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "mmu.h"
#include "i8259.h"
#include "arch.h"
#include "ffconf.h"
#include "fatfs.h"
#include "loader.h"
#include "asm_macros.h"
#include "interrupt.h"
// #define TEST_THREAD_ONLY
#define configUSE_I8259 1
static void task_user(void *arg)
{
    (void)arg;
    printf("task_user started\n");

    for (;;) {
        TickType_t tickCount = sched_task_get_tick_count();
        if( tickCount % 100 == 0 )  /* Print every 100 ticks. */
        {
             printf( "task_user Tick: %lu cpl=%d\n", ( unsigned long ) tickCount, get_cpl() );
        }
        sched_delay(100);
    }
}

static void init_task_main(void *arg)
{
    (void)arg;
    // printf("init task started\n");

    // struct task *task = task_create("task_user", task_user,
    //     TASK_TYPE_USER_PROCESS, USER_STACK_SIZE / sizeof(StackType_t), NULL);

    // if (task == NULL) {
    //     return;
    // }

    for (;;) {
         TickType_t tickCount = sched_task_get_tick_count();
         if( tickCount % 100 == 0 )  /* Print every 100 ticks. */
         {
             printf( "init_task_main Tick: %lu cpl=%d\n", ( unsigned long ) tickCount, get_cpl() );
         }
         sched_delay(100);
     }
}

int kernel_main()
{
    portDISABLE_INTERRUPTS();
    printf("Welcome to Zelix OS!\n");
    STATIC_ASSERT(sizeof(struct trap_frame) == TRAP_FRAME_SIZE);

    /* Initialise the Global Descriptor Table (GDT). */
    init_gdt();

    init_tss( 0 );

    /* Load the TSS into the task register. */
    tss_load();
    /* Initialise Interrupt Descriptor Table (IDT). */
    init_idt();

#if (configUSE_I8259 == 1)
    i8259_init();
    timer_init(100);
#endif /* configUSE_I8259 */
    fatfs_test();
    // mmu_init();
    /*init_mm();
    init_fs();
    init_syscall();
    init_sched_adapter();*/
    /* 2. 创建 init task 并启动任务系统 */
#ifdef TEST_THREAD_ONLY
    if (task_system_start("init", init_task_main, TASK_TYPE_KERNEL_THREAD, USER_STACK_SIZE / sizeof(StackType_t), NULL) == NULL) {
#else
    if (task_system_start("user_task.elf", init_task_main, TASK_TYPE_USER_PROCESS, USER_STACK_SIZE / sizeof(StackType_t), NULL) == NULL) {
#endif
        while (1) {
            /* fatal */
        }
    }
    /* 不应该到这里 */
    while (1);
    return 0;
}
