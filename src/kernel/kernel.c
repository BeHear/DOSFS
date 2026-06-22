#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "../drivers/vga.h"
#include "../drivers/serial.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../memory/pmm.h"
#include "../memory/vmm.h"
#include "../memory/heap.h"
#include "../process/scheduler.h"
#include "../process/ipc.h"
#include "../syscall/syscall.h"
#include "../fs/tmpfs.h"
#include "../shell/shell.h"
#include "../include/io.h"
#include "../libc/string.h"

static void print_banner(void) {
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("\n");
    vga_puts("  _____   _   _  _____  ____    ____  _   _ \n");
    vga_puts(" |  _  \\ | | | ||  ___||  _ \\  / ___|| | | |\n");
    vga_puts(" | | | | | |_| || |_   | | | || |    | |_| |\n");
    vga_puts(" | | | | |  _  ||  _|  | | | || |    |  _  |\n");
    vga_puts(" | |_| | | | | || |    | |_| || |___ | | | |\n");
    vga_puts(" |_____/ |_| |_||_|    |____/  \\____||_| |_|\n");
    vga_puts("\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  Microkernel v1.2\n");
    vga_puts("  (c) 2025 DanyaOS Project\n\n");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("  Initializing subsystems...\n");
}

static void init_subsystems(void) {
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

    serial_puts("[init] GDT...");
    gdt_init();
    vga_puts("  [ OK ] GDT\n");
    serial_puts(" done\n");

    serial_puts("[init] IDT...");
    idt_init();
    vga_puts("  [ OK ] IDT\n");
    serial_puts(" done\n");

    serial_puts("[init] PMM...");
    pmm_init(16 * 1024 * 1024);
    vga_puts("  [ OK ] PMM (16MB)\n");
    serial_puts(" done\n");

    serial_puts("[init] VMM...");
    vmm_init();
    vga_puts("  [ OK ] VMM\n");
    serial_puts(" done\n");

    serial_puts("[init] Heap...");
    heap_init();
    vga_puts("  [ OK ] Heap\n");
    serial_puts(" done\n");

    serial_puts("[init] Timer...");
    timer_init(100);
    vga_puts("  [ OK ] Timer (100Hz)\n");
    serial_puts(" done\n");

    serial_puts("[init] Keyboard...");
    keyboard_init();
    vga_puts("  [ OK ] Keyboard\n");
    serial_puts(" done\n");

    serial_puts("[init] Scheduler...");
    scheduler_init();
    vga_puts("  [ OK ] Scheduler\n");
    serial_puts(" done\n");

    serial_puts("[init] IPC...");
    ipc_init();
    vga_puts("  [ OK ] IPC\n");
    serial_puts(" done\n");

    serial_puts("[init] Syscalls...");
    syscall_init();
    vga_puts("  [ OK ] Syscalls\n");
    serial_puts(" done\n");

    serial_puts("[init] tmpfs...");
    tmpfs_init();
    vga_puts("  [ OK ] tmpfs\n");
    serial_puts(" done\n");

    sti();
    serial_puts("[init] All subsystems initialized!\n");
}

void kernel_main(void) {
    serial_init();
    serial_puts("\n=== DanyaOS v1.2 ===\n");
    serial_puts("[kernel] kernel_main entered\n");

    vga_init();
    serial_puts("[kernel] vga_init done\n");

    print_banner();
    serial_puts("[kernel] banner printed\n");

    init_subsystems();
    serial_puts("[kernel] subsystems initialized\n");

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("\n  All subsystems initialized successfully!\n\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  Type 'help' for available commands.\n\n");
    serial_puts("[kernel] entering shell\n");

    shell_init();
    shell_run();

    while (1) {
        hlt();
    }
}
