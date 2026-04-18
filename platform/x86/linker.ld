/* linker_high.ld - i486 higher-half kernel with embedded user blobs */

OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)

KERNEL_VMA = 0xC0000000;   /* Virtual base address. */
KERNEL_LMA = 0x00100000;   /* Physical load address. */

PHDRS
{
    boot PT_LOAD FLAGS(5);
    text PT_LOAD FLAGS(5);
    bootdata PT_LOAD FLAGS(6);
    data PT_LOAD FLAGS(6);
    stack PT_GNU_STACK FLAGS(6);
}

SECTIONS
{
    /* ========================================
     * 1. Boot section: runs at physical addresses.
     * ======================================== */
    . = KERNEL_LMA;

    .boot ALIGN(4K) : AT(KERNEL_LMA)
    {
        KEEP(*(.multiboot))
        *(.boot)
        *(.boot.text)
    } :boot

    .bootdata ALIGN(4K) : AT(ADDR(.bootdata))
    {
        *(.bootdata)
    } :bootdata

    /* ========================================
     * 2. Switch to high-half virtual addresses.
     * ======================================== */
    . += KERNEL_VMA;

    /* -------------------- */
    /* Kernel code and read-only data */
    /* -------------------- */
    .text ALIGN(4K) : AT(ADDR(.text) - KERNEL_VMA)
    {
        *(.text*)
        *(.rodata*)
        *(.eh_frame)
    } :text

    /* ----------------------------------------------------------
     * 3. User program binary blobs (User Blobs)
     * Store user_task.bin imported via .incbin here.
     * To the kernel these are data blobs, not direct mappings to 0x08048000.
     * ---------------------------------------------------------- */
    .user_blobs ALIGN(4K) : AT(ADDR(.user_blobs) - KERNEL_VMA)
    {
        _user_blobs_start = .;
        *(.user_bin)   /* Corresponds to .section .user_bin in assembly. */
        . = ALIGN(4K);
        _user_blobs_end = .;
    } :data

    /* -------------------- */
    /* Kernel data section */
    /* -------------------- */
    .data ALIGN(4K) : AT(ADDR(.data) - KERNEL_VMA)
    {
        __data_start = .;
        *(.data*)
        __data_end = .;
    } :data

    .got ALIGN(4K) : AT(ADDR(.got) - KERNEL_VMA)
    {
        *(.got)
        *(.got.plt)
    } :data

    /* -------------------- */
    /* Kernel BSS section */
    /* -------------------- */
    .bss ALIGN(4K) : AT(ADDR(.bss) - KERNEL_VMA)
    {
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        __bss_end = .;
    } :data

    /* -------------------- */
    /* Kernel heap and stack */
    /* -------------------- */
    . = ALIGN(4K);
    __heap_start = .;
    .heap : AT(ADDR(.heap) - KERNEL_VMA)
    {
        *(.heap)
        . += 64K;  /* Reserve 64KB heap space. */
    } :data
    __heap_end = .;

    . = ALIGN(16);
    __stack_bottom = .;
    . += 64K;      /* Reserve 64KB kernel stack space. */
    __stack_top = .;

    /* Export symbols for physical memory management. */
    _kernel_end = .;
    __phys_start = KERNEL_LMA;
    __phys_end = . - KERNEL_VMA;
}
