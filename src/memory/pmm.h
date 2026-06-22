#ifndef DANYA_PMM_H
#define DANYA_PMM_H

#include "types.h"

void pmm_init(uint32_t total_memory);
void* pmm_alloc_page(void);
void  pmm_free_page(void* page);
uint32_t pmm_get_free_count(void);
uint32_t pmm_get_total_count(void);

#endif
