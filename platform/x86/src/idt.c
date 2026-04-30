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
    IDTPointer_t xIDT;

    extern void exc0(); // Division by zero exception
    extern void exc8();
    extern void exc13(); // General Protection Fault (GPF)
    extern void exc32();
    extern void exc33();
    extern void exc34();
    extern void exc35();
    extern void exc36();
    extern void exc37();
    extern void exc38();
    extern void exc39();
    extern void exc40();
    extern void exc41();
    extern void exc42();
    extern void exc43();
    extern void exc44();
    extern void exc45();
    extern void exc46();
    extern void exc47();
    ISR_Handler_t const pxIRQStubs[] = {
        ( ISR_Handler_t ) exc32, ( ISR_Handler_t ) exc33,
        ( ISR_Handler_t ) exc34, ( ISR_Handler_t ) exc35,
        ( ISR_Handler_t ) exc36, ( ISR_Handler_t ) exc37,
        ( ISR_Handler_t ) exc38, ( ISR_Handler_t ) exc39,
        ( ISR_Handler_t ) exc40, ( ISR_Handler_t ) exc41,
        ( ISR_Handler_t ) exc42, ( ISR_Handler_t ) exc43,
        ( ISR_Handler_t ) exc44, ( ISR_Handler_t ) exc45,
        ( ISR_Handler_t ) exc46, ( ISR_Handler_t ) exc47,
    };
    uint32_t ulNum;

    prvSetInterruptGate(0, (ISR_Handler_t)exc0, portIDT_FLAGS);
    prvSetInterruptGate(8, (ISR_Handler_t)exc8, portIDT_FLAGS);
    prvSetInterruptGate(13, (ISR_Handler_t)exc13, portIDT_FLAGS);

    /* Install default stubs for legacy PIC IRQs (vectors 32..47). */
    for( ulNum = 0; ulNum < ( sizeof( pxIRQStubs ) / sizeof( pxIRQStubs[ 0 ] ) ); ulNum++ )
    {
        prvSetInterruptGate( ( uint8_t ) ( 32U + ulNum ), pxIRQStubs[ ulNum ], portIDT_FLAGS );
    }

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
