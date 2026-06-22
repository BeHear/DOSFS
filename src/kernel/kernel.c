#include "kernel.h"
#include "gdt.h"
#include "idt.h"
#include "../drivers/vga.h"
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
    vga_puts("  Microkernel v1.0.0\n");
    vga_puts("  (c) 2025 DanyaOS Project\n\n");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("  Initializing subsystems...\n");
}

static void init_subsystems(void) {
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] GDT\n");
    gdt_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] IDT\n");
    idt_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] PMM (16MB)\n");
    pmm_init(16 * 1024 * 1024);

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] VMM\n");
    vmm_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] Heap\n");
    heap_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] Timer (100Hz)\n");
    timer_init(100);

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] Keyboard\n");
    keyboard_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] Scheduler\n");
    scheduler_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] IPC\n");
    ipc_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] Syscalls\n");
    syscall_init();

    vga_puts("  [");
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts(" OK ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("] tmpfs\n");
    tmpfs_init();

    sti();
}

void kernel_main(void) {
    vga_init();
    print_banner();
    init_subsystems();

    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("\n  All subsystems initialized successfully!\n\n");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    vga_puts("  Type 'help' for available commands.\n\n");

    shell_init();
    shell_run();

    while (1) {
        hlt();
    }
}
