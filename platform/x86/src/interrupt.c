#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>
#include "interrupt.h"
#include "io.h"

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_IMR  0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_IMR   0xA1

#define IRQ_BASE_VECTOR 32U
#define IRQ_LAST_VECTOR 47U
#define TIMER_VECTOR    32U
#define YIELD_VECTOR    33U

typedef void (*IRQ_Handler_t)( struct exc_regs *r );

static bool xUnexpectedIRQMasked[ 16 ];
static IRQ_Handler_t pxIRQHandlers[ 16 ] = { NULL };

void register_irq_handler( uint32_t ulIRQ, IRQ_Handler_t pxHandler )
{
    if( ulIRQ < 16U )
    {
        pxIRQHandlers[ ulIRQ ] = pxHandler;
    }
}

void unregister_irq_handler( uint32_t ulIRQ )
{
    if( ulIRQ < 16U )
    {
        pxIRQHandlers[ ulIRQ ] = NULL;
    }
}

static void prvUnmaskIRQ( uint32_t ulVector )
{
    uint32_t ulIRQ = ulVector - IRQ_BASE_VECTOR;
    uint8_t ucMaskBit = ( uint8_t ) ( 1U << ( ulIRQ & 7U ) );

    if( ulIRQ >= 16U )
    {
        return;
    }

    if( ulIRQ >= 8U )
    {
        outb( PIC_SLAVE_IMR, inb( PIC_SLAVE_IMR ) & ~ucMaskBit );
    }
    else
    {
        outb( PIC_MASTER_IMR, inb( PIC_MASTER_IMR ) & ~ucMaskBit );
    }
}

static void prvMaskUnexpectedIRQ( uint32_t ulVector )
{
    uint32_t ulIRQ = ulVector - IRQ_BASE_VECTOR;
    uint8_t ucMaskBit = ( uint8_t ) ( 1U << ( ulIRQ & 7U ) );

    if( ulIRQ >= 16U )
    {
        return;
    }

    if( xUnexpectedIRQMasked[ ulIRQ ] )
    {
        return;
    }

    xUnexpectedIRQMasked[ ulIRQ ] = true;

    if( ulIRQ >= 8U )
    {
        outb( PIC_SLAVE_IMR, inb( PIC_SLAVE_IMR ) | ucMaskBit );
    }
    else
    {
        outb( PIC_MASTER_IMR, inb( PIC_MASTER_IMR ) | ucMaskBit );
    }

    printf( "Masked unexpected PIC IRQ %u (vector %u)\n", ( unsigned ) ulIRQ, ( unsigned ) ulVector );
}

void dump_stack(uint32_t *esp)
{
    printf("ESP=%p\n", esp);

    for (int i = 0; i < 10; i++) {
        printf("[%d] = %08x\n", i, esp[i]);
    }
}
/* Exception handler implementation */
void exception_handler(struct exc_regs *r) {
    if (r->vector >= IRQ_BASE_VECTOR && r->vector <= IRQ_LAST_VECTOR) {
        /*
         * Check if a handler is registered for this IRQ.
         * Timer and yield vectors are expected and handled by dedicated paths,
         * so those will not reach here.
         * Any other legacy PIC IRQ without a registered handler will be masked.
         */
        uint32_t ulIRQ = r->vector - IRQ_BASE_VECTOR;
        if( ulIRQ < 16U && pxIRQHandlers[ ulIRQ ] != NULL )
        {
            /* Call the registered handler */
            pxIRQHandlers[ ulIRQ ]( r );
        }
        else if( r->vector != TIMER_VECTOR && r->vector != YIELD_VECTOR )
        {
            /* No handler registered; mask this IRQ to prevent storm */
            prvMaskUnexpectedIRQ( r->vector );
        }

        if (r->vector >= 40) {
            // If from slave PIC (IRQ8-15), send EOI to slave as well
            outb(PIC_SLAVE_CMD, 0x20);
        }
        // Always send EOI to master PIC
        outb(PIC_MASTER_CMD, 0x20);
        return;
    }

    // Print exception information
    printf("Exception Handler - Vector: %d, Error Code: %d\n", r->vector, r->err_code);
    if( r->vector == 14U )
    {
        uint32_t fault_addr;
        __asm__ volatile ( "mov %%cr2, %0" : "=r" ( fault_addr ) );
        printf( "  Page Fault: CR2=0x%x\n", fault_addr );
    }
    printf("  Segments: CS=0x%x DS=0x%x ES=0x%x GS=0x%x\n", r->cs, r->ds, r->es, r->gs);
    printf("  Execution: EIP=0x%x EFLAGS=0x%x\n", r->eip, r->eflags);
    printf("  User Stack: ESP=0x%x SS=0x%x\n", r->user_esp, r->user_ss);
}

uint32_t get_cpu_cpl(){
    uint32_t cpl;
    __asm__ volatile (
        "mov %%cs, %0"
        : "=r" (cpl)
    );
    return cpl & 0x3; // CPL is in the lower 2 bits of CS
}
