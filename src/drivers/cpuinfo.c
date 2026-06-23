#include "cpuinfo.h"
#include "../libc/string.h"

/* CPUID: EAX=leaf, returns {eax, ebx, ecx, edx} */
static inline void cpuid(uint32_t leaf, uint32_t* eax, uint32_t* ebx,
                          uint32_t* ecx, uint32_t* edx) {
    asm volatile("cpuid"
        : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx)
        : "a"(leaf));
}

/* Read CPU brand string using extended leaves 0x80000002..0x80000004 */
static void read_brand(char* buf) {
    uint32_t max_ext, ebx, ecx, edx;
    cpuid(0x80000000, &max_ext, &ebx, &ecx, &edx);
    if (max_ext < 0x80000004) {
        buf[0] = '\0';
        return;
    }
    uint32_t* dst = (uint32_t*)buf;
    cpuid(0x80000002, &dst[0], &dst[1], &dst[2], &dst[3]);
    cpuid(0x80000003, &dst[4], &dst[5], &dst[6], &dst[7]);
    cpuid(0x80000004, &dst[8], &dst[9], &dst[10], &dst[11]);
    buf[48] = '\0';
}

void cpuinfo_detect(cpu_info_t* out) {
    uint32_t eax, ebx, ecx, edx;

    memset(out, 0, sizeof(cpu_info_t));

    /* Vendor string from leaf 0 */
    cpuid(0, &eax, &ebx, &ecx, &edx);
    out->max_leaf = eax;
    uint32_t* v = (uint32_t*)out->vendor;
    v[0] = ebx; v[1] = edx; v[2] = ecx;
    out->vendor[12] = '\0';

    /* Processor info from leaf 1 */
    if (eax >= 1) {
        cpuid(1, &eax, &ebx, &ecx, &edx);
        out->stepping  = eax & 0xF;
        out->model     = (eax >> 4) & 0xF;
        out->family    = (eax >> 8) & 0xF;
        out->ext_model = (eax >> 16) & 0xF;
        out->ext_family = (eax >> 20) & 0xFF;
        out->features_edx = edx;
        out->features_ecx = ecx;
    }

    /* Brand string */
    read_brand(out->brand);
}
