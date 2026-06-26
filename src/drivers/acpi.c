#include "acpi.h"
#include "../include/io.h"
#include "../drivers/vga.h"
#include "../libc/string.h"

static acpi_rsdp_t* rsdp = NULL;
static acpi_fadt_t* fadt = NULL;

static uint32_t find_rsdp_in_range(uint32_t start, uint32_t length) {
    uint8_t* ptr = (uint8_t*)start;
    for (uint32_t i = 0; i < length; i += 16) {
        if (memcmp(ptr + i, "RSD PTR ", 8) == 0) {
            uint8_t sum = 0;
            for (int j = 0; j < 20; j++) {
                sum += ptr[i + j];
            }
            if (sum == 0) {
                return (uint32_t)(ptr + i);
            }
        }
    }
    return 0;
}

static uint32_t find_table(uint32_t rsdt_addr, const char* sig) {
    if (!rsdt_addr) return 0;
    // RSDT must be within mapped memory (first 64MB)
    if (rsdt_addr > 0x04000000) return 0;
    acpi_sdt_header_t* rsdt = (acpi_sdt_header_t*)rsdt_addr;
    if (rsdt->length < sizeof(acpi_sdt_header_t) || rsdt->length > 0x10000) return 0;
    uint32_t entries = (rsdt->length - sizeof(acpi_sdt_header_t)) / 4;
    uint32_t* tables = (uint32_t*)(rsdt_addr + sizeof(acpi_sdt_header_t));
    for (uint32_t i = 0; i < entries; i++) {
        if (tables[i] < 0x1000 || tables[i] > 0x04000000) continue;
        acpi_sdt_header_t* table = (acpi_sdt_header_t*)tables[i];
        if (memcmp(table->signature, sig, 4) == 0) {
            return tables[i];
        }
    }
    return 0;
}

int acpi_init(void) {
    // Scan BIOS EBDA and ROM area for RSDP
    // Use conservative bounds to avoid page faults
    uint32_t rsdp_addr = 0;

    // Check EBDA segment from BIOS data area at physical 0x40E
    {
        uint16_t ebda_seg = 0;
        // Use inline asm to read from absolute address without compiler warnings
        asm volatile("mov %1, %%esi; lodsw; mov %%ax, %0"
            : "=r"(ebda_seg)
            : "r"((uint32_t)0x40E)
            : "esi", "ax", "memory");
        if (ebda_seg != 0) {
            uint32_t ebda_addr_val = (uint32_t)ebda_seg << 4;
            // EBDA is typically between 0x80000-0x9FFFF (within identity-mapped region)
            if (ebda_addr_val >= 0x80000 && ebda_addr_val <= 0x9F000) {
                rsdp_addr = find_rsdp_in_range(ebda_addr_val, 0x400);
            }
        }
    }

    // Scan BIOS ROM area
    if (!rsdp_addr)
        rsdp_addr = find_rsdp_in_range(0xE0000, 0x20000);

    if (!rsdp_addr) {
        vga_puts("  [SKIP] ACPI: RSDP not found\n");
        return -1;
    }

    rsdp = (acpi_rsdp_t*)rsdp_addr;
    uint32_t fadt_addr = find_table(rsdp->rsdt_addr, "FACP");

    if (fadt_addr) {
        fadt = (acpi_fadt_t*)fadt_addr;
        vga_printf("  [ OK ] ACPI: FADT found (PM1a=0x%x)\n", fadt->pm1a_cnt_blk);
        return 0;
    }

    vga_puts("  [SKIP] ACPI: FADT not found\n");
    return -1;
}

void acpi_shutdown(void) {
    if (fadt && fadt->pm1a_cnt_blk) {
        uint16_t val = inw(fadt->pm1a_cnt_blk);
        val &= 0xE3FF;
        val |= (7 << 10);
        val |= 0x2000;
        outw(fadt->pm1a_cnt_blk, val);

        if (fadt->pm1b_cnt_blk) {
            val = inw(fadt->pm1b_cnt_blk);
            val &= 0xE3FF;
            val |= (7 << 10);
            val |= 0x2000;
            outw(fadt->pm1b_cnt_blk, val);
        }
    }

    outb(0x64, 0xFE);
    cli();
    hlt();
}

void acpi_reboot(void) {
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);
    cli();
    hlt();
}

acpi_rsdp_t* acpi_get_rsdp(void) {
    return rsdp;
}

acpi_fadt_t* acpi_get_fadt(void) {
    return fadt;
}
