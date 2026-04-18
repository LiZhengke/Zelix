# Zelix Development Guide

This guide documents low-level implementation notes for the x86 + FreeRTOS
kernel port.

## Scope

Use this document for:
- FreeRTOS port behavior on i486
- interrupt and timer flow
- context switching internals
- debugging reset loops and scheduler issues

Use the top-level README for build and run quick-start.

## Important Paths

- `platform/x86/inc/FreeRTOSConfig.h`
- `platform/x86/inc/portmacro.h`
- `platform/x86/port/port.c`
- `platform/x86/port/portasm.S`
- `platform/x86/src/i8259.c`
- `platform/x86/src/idt.c`
- `platform/x86/src/interrupt.c`
- `kernel/task/src/ktask.c`
- `kernel/sched/src/adapter.c`

## Interrupt and Tick Flow

1. `kernel/main.c` initializes GDT, TSS, IDT, PIC, and PIT.
2. PIT IRQ0 is mapped to vector `0x20`.
3. `vPortTimerHandler` runs on each tick.
4. Tick handler updates scheduler state via `xTaskIncrementTick`.
5. If a switch is needed, `vTaskSwitchContext` selects a new TCB.
6. Restored task context returns with `iret`.

## Yield Path

The software yield path must use the configured yield interrupt vector.

- `portYIELD()` triggers `int 0x21`.
- `vPortYieldCall` runs as an ISR-style entry/exit path.
- `vPortYieldCall` must save the current task ESP to `pxCurrentTCB` before
  calling `vTaskSwitchContext`.

Calling `vPortYieldCall` as a normal C function is invalid because the handler
returns using `iret`.

## PIC/PIT Notes

- `i8259_init()` remaps PIC vectors to `0x20-0x2F`.
- IRQ2 (cascade) and IRQ0 (timer) must be unmasked.
- PIT channel 0 is configured in rate generator mode.
- Timer ISR must send EOI to the master PIC to allow subsequent ticks.

## Task Stack Layout Notes

`pxPortInitialiseStack` must build a stack frame that matches the restore
sequence in assembly (`popal` + `iret`).

A mismatch usually appears as:
- boot success
- early ticks working
- reset/triple-fault at first real unblock/switch point

## Common Failure Pattern: Reset at Tick N

Symptom:
- system prints ticks normally
- resets at a deterministic tick (often when delayed tasks wake)

Likely causes:
- broken yield path (`portYIELD` not using interrupt)
- current task ESP not saved before context switch
- bad initial task frame layout
- missing PIC EOI in timer handler

## Debug Checklist

1. Confirm IRQ0 is unmasked in PIC init.
2. Confirm timer ISR sends EOI.
3. Confirm `portYIELD()` uses interrupt vector.
4. Confirm yield/tick handlers save `pxCurrentTCB` stack pointer correctly.
5. Confirm stack frame created in `pxPortInitialiseStack` matches restore
   order.
6. Reduce prints in ISR context while debugging timing-sensitive faults.

## Logging and Runtime Notes

- Use `timeout` for bounded QEMU runs when collecting logs.
- Keep ISR printing minimal because it affects timing and can hide root causes.
- Prefer periodic task-level logs for scheduler behavior verification.
