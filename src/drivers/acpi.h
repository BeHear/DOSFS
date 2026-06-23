#ifndef DANYA_ACPI_H
#define DANYA_ACPI_H

#include "../include/types.h"

typedef struct {
    char     signature[4];
    uint32_t length;
    uint8_t  revision;
    uint8_t  checksum;
    char     oem_id[6];
    char     oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    char     signature[8];
    uint8_t  checksum;
    char     oem_id[6];
    uint8_t  revision;
    uint32_t rsdt_addr;
} __attribute__((packed)) acpi_rsdp_t;

typedef struct {
    uint16_t pm1a_control;
    uint16_t pm1b_control;
    uint32_t flags;
} __attribute__((packed)) acpi_fadt_t;

int  acpi_init(void);
void acpi_shutdown(void);
void acpi_reboot(void);

#endif
