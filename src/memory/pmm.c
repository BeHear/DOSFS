#include "pmm.h"
#include "../include/io.h"
#include "../libc/string.h"
#include "../drivers/vga.h"
#include "../rust_ffi.h"

/*
 * PMM — C wrapper that delegates to the Rust implementation.
 * The Rust code in rust/src/pmm.rs provides the actual bitmap allocator
 * with improved safety (null checks, bounds validation).
 */

void pmm_init(uint32_t total_memory) {
    rust_pmm_init(total_memory);
}

void* pmm_alloc_page(void) {
    return rust_pmm_alloc_page();
}

void pmm_free_page(void* page) {
    rust_pmm_free_page(page);
}

uint32_t pmm_get_free_count(void) {
    return rust_pmm_get_free_count();
}

uint32_t pmm_get_total_count(void) {
    return rust_pmm_get_total_count();
}
