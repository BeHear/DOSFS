#include "gdt.h"
#include "../include/io.h"

static gdt_t gdt;

extern void gdt_flush(uint32_t);

static void gdt_set_gate(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt.entries[num].base_low    = base & 0xFFFF;
    gdt.entries[num].base_middle = (base >> 16) & 0xFF;
    gdt.entries[num].base_high   = (base >> 24) & 0xFF;
    gdt.entries[num].limit_low   = limit & 0xFFFF;
    gdt.entries[num].granularity  = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt.entries[num].access      = access;
}

void gdt_init(void) {
    gdt.ptr.limit = sizeof(gdt.entries) - 1;
    gdt.ptr.base  = (uint32_t)&gdt.entries;

    gdt_set_gate(0, 0, 0, 0, 0);
    gdt_set_gate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);
    gdt_set_gate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);
    gdt_set_gate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);
    gdt_set_gate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);

    gdt_flush((uint32_t)&gdt.ptr);
}
