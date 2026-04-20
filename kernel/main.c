#include <stdio.h>
#include "ktask.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "mmu.h"
#include "i8259.h"

#define configUSE_I8259 1

static void init_task_main(void *arg)
{
    (void)arg;

    printf("init task started\n");
}

int kernel_main()
{
    printf("Welcome to Zelix OS!\n");
    /* Initialise the Global Descriptor Table (GDT). */
    init_gdt();
    /* Load the TSS into the task register. */
    tss_load();
    /* Initialise Interrupt Descriptor Table (IDT). */
    init_idt();

#if (configUSE_I8259 == 1)
    i8259_init();
    timer_init(100);
#endif /* configUSE_I8259 */
    // mmu_init();
    /*init_mm();
    init_fs();
    init_syscall();
    init_sched_adapter();*/
    /* 2. 创建 init task 并启动任务系统 */
    if (task_system_start("init", init_task_main, TASK_TYPE_PROCESS, NULL) == NULL) {
        while (1) {
            /* fatal */
        }
    }
    /* 不应该到这里 */
    while (1);
    return 0;
}
