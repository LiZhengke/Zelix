#include <stdint.h>
#include <stdbool.h>
#include "io.h"
#include "i8259.h"
#include "FreeRTOS.h"
typedef uint16_t u16;
typedef uint8_t u8;
#define MASTER_PIC 0x20
// #define MASTER_PIC + IMR 0x21
#define SLAVE_PIC 0xA0
// #define SLAVE_PIC + IMR 0xA1
#define SYS_NUM_IRQS	16

#define PIT_COMMAND_PORT 0x43
#define PIT_CHANNEL0_PORT 0x40

static void mask_irq(int irq) __attribute__((unused));
static void unmask_irq(int irq) __attribute__((unused));
static void configure_irq_trigger(int int_num, bool is_level_triggered) __attribute__((unused));
static u8 dump_imr() __attribute__((unused));

static u8 dump_imr()
{
   return inb(MASTER_PIC + IMR);
}

static void mask_irq(int irq)
{
	int imr_port;

	if (irq >= SYS_NUM_IRQS)
		return;

	if (irq > 7)
		imr_port = SLAVE_PIC + IMR;
	else
		imr_port = MASTER_PIC + IMR;

	outb(imr_port,inb(imr_port) | (1 << (irq & 7)));
}

static void unmask_irq(int irq)
{
	int imr_port;

	if (irq >= SYS_NUM_IRQS)
		return;

	if (irq > 7)
		imr_port = SLAVE_PIC + IMR;
	else
		imr_port = MASTER_PIC + IMR;

	outb(imr_port,inb(imr_port) & ~(1 << (irq & 7)));
}

static void configure_irq_trigger(int int_num, bool is_level_triggered)
{
	u16 int_bits = inb(ELCR1) | (((u16)inb(ELCR2)) << 8);

	if (is_level_triggered)
		int_bits |= (1 << int_num);
	else
		int_bits &= ~(1 << int_num);

	/* Write new values */
	outb(ELCR1,(u8)(int_bits & 0xff));
	outb(ELCR2,(u8)(int_bits >> 8));
}

void i8259_init(void) {
    int i = 0;

    // 1. mask all ints
    outb(MASTER_PIC + IMR,0xff);
    outb(SLAVE_PIC + IMR,0xff);

    // 2. Initialize ICW1: Cascade mode, ICW4 needed
    outb(MASTER_PIC + ICW1, ICW1_SEL | ICW1_EICW4);
    outb(MASTER_PIC + ICW2,0x20);
	outb(MASTER_PIC + ICW3,IR2);
	outb(MASTER_PIC + ICW4, ICW4_PM);

	for (i = 0; i < 8; i++)
		outb(MASTER_PIC + OCW2,OCW2_SEOI | i);
    /*
	 * Slave PIC
	 * Place slave PIC interrupts at INT28
	 */
	outb(SLAVE_PIC + ICW1,ICW1_SEL | ICW1_EICW4);
	outb(SLAVE_PIC + ICW2,0x28);
	outb(SLAVE_PIC + ICW3,0x02);
	outb(SLAVE_PIC + ICW4,ICW4_PM);

	for (i = 0; i < 8; i++)
		outb(SLAVE_PIC + OCW2,OCW2_SEOI | i);

	/*
	 * Enable cascaded interrupts by unmasking the cascade IRQ pin of
	 * the master PIC
	 */
	unmask_irq(2);
	unmask_irq(0);

	/* Interrupt 9 should be level triggered (SCI). The OS might do this */
	configure_irq_trigger(9, true);

    // 6. Restore mask or enable all interrupts (0x00 enable all, 0xFB enable clock and cascade)
    // outb(MASTER_PIC + IMR, 0x00);
    // outb(SLAVE_PIC + IMR, 0x00);
}

void timer_init(uint32_t frequency) {
    // 1. Calculate divisor
    uint32_t divisor = 1193182 / frequency;

    // 2. Send control word to PIT command port
    // 0x34 meaning:
    // 00 (channel 0) | 11 (access mode: lobyte/hibyte) | 011 (mode 2: rate generator) | 0 (binary counting)
    outb(PIT_COMMAND_PORT, 0x34);

    // 3. Write the divisor (must be written in two 8-bit port writes)
    uint8_t low = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);

    outb(PIT_CHANNEL0_PORT, low);
    outb(PIT_CHANNEL0_PORT, high);
}
