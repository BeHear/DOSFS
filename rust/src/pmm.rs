/// Physical Memory Manager — bitmap-based page allocator
/// Migrated from src/memory/pmm.c

use core::sync::atomic::{AtomicBool, AtomicU32, Ordering};

const BITMAP_SIZE: usize = 8192;
const PAGE_SIZE: u32 = 4096;

static mut BITMAP: [u8; BITMAP_SIZE] = [0u8; BITMAP_SIZE];
static TOTAL_PAGES: AtomicU32 = AtomicU32::new(0);
static USED_PAGES: AtomicU32 = AtomicU32::new(0);
static INITIALIZED: AtomicBool = AtomicBool::new(false);

#[inline]
fn bitmap_set(bit: u32) {
    unsafe {
        let ptr = (&raw mut BITMAP as *mut u8).add((bit / 8) as usize);
        *ptr |= 1 << (bit % 8);
    }
}

#[inline]
fn bitmap_clear(bit: u32) {
    unsafe {
        let ptr = (&raw mut BITMAP as *mut u8).add((bit / 8) as usize);
        *ptr &= !(1 << (bit % 8));
    }
}

#[inline]
fn bitmap_test(bit: u32) -> bool {
    unsafe {
        let ptr = (&raw const BITMAP as *const u8).add((bit / 8) as usize);
        *ptr & (1 << (bit % 8)) != 0
    }
}

/// # Safety
/// Must be called once during kernel boot with the correct memory size.
/// Accesses static mutable bitmap — safe only during single-threaded init.
#[no_mangle]
pub unsafe extern "C" fn rust_pmm_init(total_memory: u32) {
    let tp = total_memory / PAGE_SIZE;
    let tp = if tp > (BITMAP_SIZE * 8) as u32 {
        (BITMAP_SIZE * 8) as u32
    } else {
        tp
    };
    TOTAL_PAGES.store(tp, Ordering::Relaxed);

    // Zero the bitmap via raw pointer to avoid mutable static ref warning
    let bitmap_ptr = &raw mut BITMAP as *mut u8;
    core::ptr::write_bytes(bitmap_ptr, 0, BITMAP_SIZE);

    // Mark kernel pages as used (symbol from linker script)
    extern "C" {
        static __kernel_end: u8;
    }
    let kernel_end = &__kernel_end as *const u8 as u32;
    let reserved_pages = (kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for i in 0..reserved_pages {
        bitmap_set(i);
    }

    // Mark bitmap's own pages as used
    let bitmap_pages = (BITMAP_SIZE as u32 + PAGE_SIZE - 1) / PAGE_SIZE;
    for i in 0..bitmap_pages {
        bitmap_set(i);
    }

    // Mark VGA text buffer as used
    let vga_start = 0xB8000 / PAGE_SIZE;
    let vga_end = (0xB8000 + 80 * 25 * 2 + PAGE_SIZE - 1) / PAGE_SIZE;
    for i in vga_start..vga_end {
        bitmap_set(i);
    }

    // Count used pages
    let mut used: u32 = 0;
    for i in 0..tp {
        if bitmap_test(i) {
            used += 1;
        }
    }
    USED_PAGES.store(used, Ordering::Relaxed);
    INITIALIZED.store(true, Ordering::Release);
}

/// Allocate a single physical page. Returns physical address or null (0).
///
/// # Safety
/// Caller must ensure returned address is used correctly and freed only once.
#[no_mangle]
pub extern "C" fn rust_pmm_alloc_page() -> *mut u8 {
    let was_enabled;
    unsafe {
        let flags: u32;
        core::arch::asm!("pushfd; pop {}", out(reg) flags);
        was_enabled = flags & 0x200 != 0;
        core::arch::asm!("cli");
    }

    let tp = TOTAL_PAGES.load(Ordering::Relaxed);
    for i in 0..tp {
        if !bitmap_test(i) {
            bitmap_set(i);
            USED_PAGES.fetch_add(1, Ordering::Relaxed);
            if was_enabled {
                unsafe { core::arch::asm!("sti"); }
            }
            return (i * PAGE_SIZE) as *mut u8;
        }
    }

    if was_enabled {
        unsafe { core::arch::asm!("sti"); }
    }
    core::ptr::null_mut()
}

/// Free a previously allocated page.
///
/// # Safety
/// `page` must be a previously allocated page address. Double-free is silently ignored.
#[no_mangle]
pub extern "C" fn rust_pmm_free_page(page: *mut u8) {
    if page.is_null() {
        return;
    }
    let index = page as u32 / PAGE_SIZE;
    let tp = TOTAL_PAGES.load(Ordering::Relaxed);
    if index < tp && bitmap_test(index) {
        bitmap_clear(index);
        USED_PAGES.fetch_sub(1, Ordering::Relaxed);
    }
}

/// Get count of free pages.
#[no_mangle]
pub extern "C" fn rust_pmm_get_free_count() -> u32 {
    let tp = TOTAL_PAGES.load(Ordering::Relaxed);
    let up = USED_PAGES.load(Ordering::Relaxed);
    tp.saturating_sub(up)
}

/// Get total page count.
#[no_mangle]
pub extern "C" fn rust_pmm_get_total_count() -> u32 {
    TOTAL_PAGES.load(Ordering::Relaxed)
}
