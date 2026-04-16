#include <stdint.h>
#include "gdt.h"
#include "tss.h"

// GDT entry structure (8 bytes)
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

// GDTR pointer structure (6 bytes)
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Must be defined as global or static to prevent stack space from being overwritten after function exit
static struct gdt_entry gdt[6];
static struct gdt_ptr   gp;

// Global TSS instance
extern struct tss tss_entry;

// External assembly function: used to load GDTR and flush segment registers
extern void gdt_flush(uint32_t gdt_ptr_addr);

void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F);

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void init_gdt() {
    // 1. Initialize the GDTR (Global Descriptor Table Register) structure
    // The limit is the size of the GDT minus 1 (6 entries * 8 bytes each = 48 bytes, so limit = 47)
    gp.limit = (sizeof(struct gdt_entry) * 6) - 1;
    // Base address points to the start of our GDT array in memory
    gp.base  = (uint32_t)&gdt;

    // 2. Null descriptor (required by x86 architecture, must be first entry at index 0)
    // All fields are zero - any attempt to use this selector will cause a General Protection Fault
    gdt_set_gate(0, 0, 0, 0, 0);

    // 3. Kernel mode code segment (index 1, selector 0x08)
    // Base: 0x00000000, Limit: 0xFFFFFFFF (4GB with 4KB granularity)
    // Access: 0x9A = 10011010b
    //   Bit 7: Present (P=1, segment is present in memory)
    //   Bits 5-6: Descriptor Privilege Level (DPL=00b, ring 0/kernel mode)
    //   Bit 4: Descriptor type (S=1, code/data segment)
    //   Bits 0-3: Type (1010b = code segment, executable, readable, not accessed)
    // Granularity: 0xCF = 11001111b
    //   Bit 7: Granularity (G=1, limit is in 4KB blocks)
    //   Bit 6: Size (D/B=1, 32-bit protected mode segment)
    //   Bits 0-3: Upper 4 bits of limit
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);

    // 4. Kernel mode data segment (index 2, selector 0x10)
    // Base: 0x00000000, Limit: 0xFFFFFFFF (4GB with 4KB granularity)
    // Access: 0x92 = 10010010b
    //   Bit 7: Present (P=1)
    //   Bits 5-6: DPL=00b (ring 0/kernel mode)
    //   Bit 4: S=1 (code/data segment)
    //   Bits 0-3: Type (0010b = data segment, writable, not accessed)
    // Granularity: 0xCF (same as code segment - 4GB, 32-bit, 4KB blocks)
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);

    // 5. User mode code segment (index 3, selector 0x18 + RPL 3 = 0x1B)
    // Base: 0x00000000, Limit: 0xFFFFFFFF (4GB with 4KB granularity)
    // Access: 0xFA = 11111010b
    //   Bit 7: Present (P=1, segment is present in memory)
    //   Bits 5-6: Descriptor Privilege Level (DPL=11b, ring 3/user mode)
    //   Bit 4: Descriptor type (S=1, code/data segment)
    //   Bits 0-3: Type (1010b = code segment, executable, readable, not accessed)
    // Granularity: 0xCF = 11001111b
    //   Bit 7: Granularity (G=1, limit is in 4KB blocks)
    //   Bit 6: Size (D/B=1, 32-bit protected mode segment)
    //   Bits 0-3: Upper 4 bits of limit
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);

    // 6. User mode data segment (index 4, selector 0x20 + RPL 3 = 0x23)
    // Base: 0x00000000, Limit: 0xFFFFFFFF (4GB with 4KB granularity)
    // Access: 0xF2 = 11110010b
    //   Bit 7: Present (P=1, segment is present in memory)
    //   Bits 5-6: Descriptor Privilege Level (DPL=11b, ring 3/user mode)
    //   Bit 4: Descriptor type (S=1, code/data segment)
    //   Bits 0-3: Type (0010b = data segment, writable, not accessed)
    // Granularity: 0xCF (same as code segment - 4GB, 32-bit, 4KB blocks)
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    // 7. Task State Segment (TSS) descriptor (index 5/TSS_INDEX, selector 0x28)
    // Base: address of tss_entry structure
    // Limit: sizeof(struct tss) - 1 (TSS size minus 1, typically 103 bytes for 32-bit TSS)
    // Access: 0x89 = 10001001b
    //   Bit 7: Present (P=1, TSS is present in memory)
    //   Bits 5-6: Descriptor Privilege Level (DPL=00b, ring 0/kernel access only)
    //   Bit 4: Descriptor type (S=0, system segment, not code/data)
    //   Bits 0-3: Type (1001b = 32-bit available TSS, not busy)
    // Granularity: 0x00 (G=0, limit is in bytes; TSS descriptors don't use 4KB granularity)
    gdt_set_gate(TSS_INDEX, tss_get_address(), tss_get_size() - 1, 0x89, 0x00);

    // 5. Call assembly to flush
    gdt_flush((uint32_t)&gp);
}
