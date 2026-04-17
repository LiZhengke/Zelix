#include "idt.h"
#include "limits.h"

/*
 * Handler for portYIELD().
 */
extern void vPortYieldCall( void );

/*
 * Configure the APIC to generate the RTOS tick.
 */
static void prvSetupTimerInterrupt( void );

/*
 * Tick interrupt handler.
 */
extern void vPortTimerHandler( void );

/* Handler for portSYSCALL(). */
extern void vPortSysCall( void );

/* The IDT itself. */
static __attribute__( ( aligned( 32 ) ) ) IDTEntry_t xInterruptDescriptorTable[ portNUM_VECTORS ];

static void prvSetInterruptGate( uint8_t ucNumber,
                                 ISR_Handler_t pxHandlerFunction,
                                 uint8_t ucFlags )
{
    uint16_t usCodeSegment;
    uint32_t ulBase = ( uint32_t ) pxHandlerFunction;

    xInterruptDescriptorTable[ ucNumber ].usISRLow = ( uint16_t ) ( ulBase & USHRT_MAX );
    xInterruptDescriptorTable[ ucNumber ].usISRHigh = ( uint16_t ) ( ( ulBase >> 16UL ) & USHRT_MAX );

    /* When the flat model is used the CS will never change. */
    __asm volatile ( "mov %%cs, %0" : "=r" ( usCodeSegment ) );
    xInterruptDescriptorTable[ ucNumber ].usSegmentSelector = usCodeSegment;
    xInterruptDescriptorTable[ ucNumber ].ucZero = 0;
    xInterruptDescriptorTable[ ucNumber ].ucFlags = ucFlags;
}
/*-----------------------------------------------------------*/

void init_idt( void )
{
    uint32_t ulNum;
    IDTPointer_t xIDT;

    /* Initialise each entry in the interrupt descriptor table to the same handler. */
    (void) ulNum;

    extern void exc0(); // Division by zero exception
    extern void exc13(); // General Protection Fault (GPF)
    extern void exc8();

    prvSetInterruptGate(0, (ISR_Handler_t)exc0, portIDT_FLAGS);
    prvSetInterruptGate(8, (ISR_Handler_t)exc8, portIDT_FLAGS);
    prvSetInterruptGate(13, (ISR_Handler_t)exc13, portIDT_FLAGS);
    /* Install timer handler.  */
    prvSetInterruptGate( ( uint8_t ) portAPIC_TIMER_INT_VECTOR, vPortTimerHandler, portIDT_FLAGS );

    /* Install Yield handler. */
    prvSetInterruptGate( ( uint8_t ) portAPIC_YIELD_INT_VECTOR, vPortYieldCall, portIDT_FLAGS );

    prvSetInterruptGate( ( uint8_t ) portSYSCALL_INT_VECTOR, vPortSysCall, portIDT_FLAGS_RING3 );


    /* Set IDT address. */
    xIDT.ulTableBase = ( uint32_t ) xInterruptDescriptorTable;
    xIDT.usTableLimit = sizeof( xInterruptDescriptorTable ) - 1;

    /* Set IDT in CPU. */
    __asm volatile ( "lidt %0" ::"m" ( xIDT ) );
}
