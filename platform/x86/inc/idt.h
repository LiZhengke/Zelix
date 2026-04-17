#ifndef IDT_H
#define IDT_H
#include <stdint.h>

#define portNUM_VECTORS     256
#define portMAX_PRIORITY    15
typedef void ( * ISR_Handler_t ) ( void );

/* EFLAGS bits. */
#define portEFLAGS_IF                    ( 0x200UL )

/* Default flags setting for entries in the IDT.
 * 0x8E = Present, DPL=0 (ring 0), 32-bit interrupt gate */
#define portIDT_FLAGS                    ( 0x8E )
#define portIDT_FLAGS_RING3              ( 0xEE ) /* Same as above but DPL=3 (ring 3) */

/* The interrupt priority (for vectors 16 to 255) is determined using vector/16.
 * The quotient is rounded to the nearest integer with 1 being the lowest priority
 * and 15 is the highest.  Therefore the following two interrupts are at the lowest
 * priority.  *NOTE 1* If the yield vector is changed then it must also be changed
 * in the portYIELD_INTERRUPT definition immediately below. */
#define portAPIC_TIMER_INT_VECTOR       ( 0x20 )
#define portAPIC_YIELD_INT_VECTOR       ( 0x21 )
#define portSYSCALL_INT_VECTOR          ( 0x30 )

/* A structure used to map the various fields of an IDT entry into separate
 * structure members. */
struct IDTEntry
{
    uint16_t usISRLow;          /* Low 16 bits of handler address. */
    uint16_t usSegmentSelector; /* Flat model means this is not changed. */
    uint8_t ucZero;             /* Must be set to zero. */
    uint8_t ucFlags;            /* Flags for this entry. */
    uint16_t usISRHigh;         /* High 16 bits of handler address. */
}
__attribute__( ( packed ) );
typedef struct IDTEntry IDTEntry_t;


/* Use to pass the location of the IDT to the CPU. */
struct IDTPointer
{
    uint16_t usTableLimit;
    uint32_t ulTableBase; /* The address of the first entry in xInterruptDescriptorTable. */
}
__attribute__( ( __packed__ ) );
typedef struct IDTPointer IDTPointer_t;

void init_idt(void);
#endif /* IDT_H */
