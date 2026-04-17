#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "mmu.h"

int kernel_main()
{
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
    /*init_mm();
    init_fs();
    init_syscall();
    init_sched_adapter();*/
#if 0
    /* 2. 创建 init task（用户态起点） */
    init_task = task_create("init", kernel_task_entry, NULL);

    if (!init_task) {
        while (1) {
            /* fatal */
        }
    }

    /* 3. 启动 FreeRTOS scheduler */
    vTaskStartScheduler();
#endif
    /* 不应该到这里 */
    while (1);
    return 0;
}
