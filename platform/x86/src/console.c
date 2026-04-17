#include <stdint.h>

#include "io.h"

#define COM1_PORT 0x3F8
#define UART_LCR 3
#define UART_LSR 5
#define UART_DLL 0
#define UART_DLM 1
#define UART_FCR 2
#define UART_MCR 4

#define UART_LCR_DLAB 0x80
#define UART_LCR_8N1 0x03
#define UART_LSR_THRE 0x20

static int g_serial_ready;

static void serial_init(void)
{
    outb(COM1_PORT + 1, 0x00);
    outb(COM1_PORT + UART_LCR, UART_LCR_DLAB);
    outb(COM1_PORT + UART_DLL, 0x03);
    outb(COM1_PORT + UART_DLM, 0x00);
    outb(COM1_PORT + UART_LCR, UART_LCR_8N1);
    outb(COM1_PORT + UART_FCR, 0xC7);
    outb(COM1_PORT + UART_MCR, 0x0B);

    g_serial_ready = 1;
}

void putchar(char c)
{
    if (!g_serial_ready) {
        serial_init();
    }

    if (c == '\n') {
        putchar('\r');
    }

    while ((inb(COM1_PORT + UART_LSR) & UART_LSR_THRE) == 0U) {
    }

    outb(COM1_PORT, (uint8_t)c);
}
