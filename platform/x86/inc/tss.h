#ifndef TSS_H
#define TSS_H

#include <stdint.h>
#include <gdt.h>

#define TSS_INDEX    5
#define TSS_SELECTOR  (TSS_INDEX << 3)

void init_tss(uint32_t kernel_stack_top);
void tss_load(void);
uint32_t tss_get_size(void);
uintptr_t tss_get_address(void);
void tss_set_esp0(uint32_t esp0);
uint32_t tss_get_esp0(void);
#endif /* TSS_H */
