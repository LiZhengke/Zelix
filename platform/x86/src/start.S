.global _start
.extern __stack_top
.extern kernel_main

/*
 * Higher-half kernel boot sequence:
 *
 * 1. GRUB loads us at physical 0x00100000 (1MB) with paging OFF.
 *    _start is in .boot section with VMA == LMA (physical address).
 *
 * 2. Build a minimal page directory:
 *      - Identity-map first 8MB (phys 0 → virt 0)  so current code keeps running
 *      - Map virt 0xC0000000 → phys 0               so kernel high addresses work
 *    Using 4MB PSE (Page Size Extension) pages for simplicity.
 *
 * 3. Enable paging (CR4.PSE + CR0.PG).
 *
 * 4. Jump to high-address code, remove identity map, set up stack, call C.
 */

/* ---------- Multiboot header ---------- */
.section .multiboot, "a"
.align 4
    .long 0x1BADB002                    /* magic */
    .long 0x00                          /* flags */
    .long -(0x1BADB002 + 0x00)          /* checksum */

/* ======================================================
 * Early boot code — runs at PHYSICAL addresses (no paging yet).
 * .boot section has VMA == LMA (physical).
 * ====================================================== */
.section .bootdata, "aw"
.code32

.if configUSE_PAGE_ASMBLE_CODE == 1
/*
 * Boot page directory — 4096 bytes, 4K-aligned.
 * Placed in boot data so it is available before we clear BSS.
 */
.align 4096
boot_page_directory:
    .fill 1024, 4, 0                    /* 1024 entries, all zero initially */
.endif /* configUSE_PAGE_ASMBLE_CODE */

.section .boot, "ax"
.align 4
_start:
    cli
.if configUSE_PAGE_ASMBLE_CODE == 1
    /* ----- 1. Build boot page directory (identity + high-half) ----- */
    /* boot_page_directory has VMA == LMA (physical), so use it directly */
    leal boot_page_directory, %edi

    /*
     * Entry value for a 4MB PSE page mapping phys 0x00000000:
     *   bit 7 = PS (page size = 4MB)
     *   bit 1 = R/W
     *   bit 0 = Present
     * = 0x00000083
     */
    movl $0x00000083, %eax

    /* PDE index 0   → identity map: virt 0x00000000 → phys 0x00000000 */
    movl %eax, 0(%edi)

    /* PDE index 768 → high-half:  virt 0xC0000000 → phys 0x00000000
     * index 768 = 0xC0000000 >> 22.  Offset in table = 768*4 = 3072 */
    movl %eax, 3072(%edi)

    /* Map a second 4MB page so we cover 0-8MB physical:
     *   PDE[1]   → virt 0x00400000 → phys 0x00400000
     *   PDE[769] → virt 0xC0400000 → phys 0x00400000
     */
    movl $0x00400083, %eax
    movl %eax, 4(%edi)
    movl %eax, 3076(%edi)

    /* ----- 2. Load page directory into CR3 ----- */
    movl %edi, %cr3

    /* ----- 3. Enable PSE (4MB pages) in CR4 ----- */
    movl %cr4, %eax
    orl  $0x00000010, %eax              /* CR4.PSE = bit 4 */
    movl %eax, %cr4

    /* ----- 4. Enable paging in CR0 ----- */
    movl %cr0, %eax
    orl  $0x80000000, %eax              /* CR0.PG = bit 31 */
    movl %eax, %cr0
.endif /* configUSE_PAGE_ASMBLE_CODE */
    call init_paging

    /* ----- 5. Jump to high-address code ----- */
    leal _higher_half, %eax
    jmp  *%eax

/* ======================================================
 * From here on we run at virtual addresses (0xC01xxxxx).
 * ====================================================== */
.section .text
.align 4
_higher_half:
.if configUSE_PAGE_ASMBLE_CODE == 1
    /* ----- 6. Remove identity mapping (PDE[0] and PDE[1]) ----- */
    /* boot_page_directory is in .boot (physical VMA), but thanks to
     * our high-half mapping, phys addr N is also accessible at
     * 0xC0000000 + N.  We need the physical address for the PDE
     * entries.  Since the page dir is < 8MB phys, we can access it
     * via the high-half mapping. */
    movl $boot_page_directory, %edi     /* physical address (VMA==LMA) */
    addl $0xC0000000, %edi             /* convert to high-half virtual */
    movl $0, 0(%edi)                   /* clear PDE[0] */
    movl $0, 4(%edi)                   /* clear PDE[1] */

    /* Flush TLB by reloading CR3 */
    movl %cr3, %eax
    movl %eax, %cr3
.endif /* configUSE_PAGE_ASMBLE_CODE */

    /* ----- 7. Set up kernel stack (virtual address) ----- */
    movl $__stack_top, %esp
    movl $__stack_top, %ebp

    # Optional: clear EBP to simplify stack backtracing.
    xor %ebp, %ebp
    /* ----- 8. Enter C code ----- */
    call kernel_main

halt:
    hlt
    jmp halt

.section .note.GNU-stack,"",@progbits
/*-----------------------------------------------------------*/
