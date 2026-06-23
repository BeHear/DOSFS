#ifndef DANYA_CPUINFO_H
#define DANYA_CPUINFO_H

#include "../include/types.h"

typedef struct {
    char vendor[13];
    char brand[49];
    uint32_t max_leaf;
    uint32_t stepping;
    uint32_t model;
    uint32_t family;
    uint32_t ext_model;
    uint32_t ext_family;
    uint32_t features_edx;
    uint32_t features_ecx;
    uint32_t brand_leaf;  /* 0 if unsupported */
} cpu_info_t;

void cpuinfo_detect(cpu_info_t* out);

#endif
