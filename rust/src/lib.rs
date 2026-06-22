#![no_std]
#![allow(clippy::not_unsafe_ptr_arg_deref)]

pub mod pmm;
pub mod ipc;

/// Panic handler — halt the CPU (no_std environment)
#[panic_handler]
fn panic(_info: &core::panic::PanicInfo) -> ! {
    unsafe {
        core::arch::asm!("cli");
        loop {
            core::arch::asm!("hlt");
        }
    }
}
