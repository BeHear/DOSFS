#ifndef DANYA_VMM_H
#define DANYA_VMM_H

#include "types.h"

typedef struct {
    uint32_t present : 1;
    uint32_t rw      : 1;
    uint32_t user    : 1;
    uint32_t accessed: 1;
    uint32_t dirty   : 1;
    uint32_t unused  : 7;
    uint32_t frame   : 20;
} __attribute__((packed)) page_table_entry_t;

typedef struct {
    uint32_t present : 1;
    uint32_t rw      : 1;
    uint32_t user    : 1;
    uint32_t unused  : 9;
    uint32_t table   : 20;
} __attribute__((packed)) page_directory_entry_t;

void     vmm_init(void);
void     vmm_map_page(uint32_t virt, uint32_t phys, uint32_t flags);
void     vmm_unmap_page(uint32_t virt);
void*    vmm_get_physical(uint32_t virt);
void     vmm_switch_directory(page_directory_entry_t* dir);
page_directory_entry_t* vmm_create_directory(void);
void     vmm_free_directory(page_directory_entry_t* dir);

#endif
