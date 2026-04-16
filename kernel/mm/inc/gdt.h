#ifndef KERNEL_GDT_H
#define KERNEL_GDT_H
#include <stdint.h>

#define KERNEL_CS  0x08
#define KERNEL_DS  0x10
#define USER_CS    0x1B
#define USER_DS    0x23

#define DPL_KERNEL 0
#define DPL_USER   3

void init_gdt();
#endif /* KERNEL_GDT_H */
