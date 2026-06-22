#include "shell.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../drivers/timer.h"
#include "../include/io.h"
#include "../memory/pmm.h"
#include "../memory/heap.h"
#include "../process/scheduler.h"
#include "../process/ipc.h"
#include "../fs/tmpfs.h"
#include "../libc/string.h"

#define CMD_BUF_SIZE 256

static char cmd_buf[CMD_BUF_SIZE];
static int cmd_len = 0;

static void print_prompt(void) {
    vga_set_color(VGA_LIGHT_GREEN, VGA_BLACK);
    vga_puts("danya");
    vga_set_color(VGA_WHITE, VGA_BLACK);
    vga_puts("@os");
    vga_set_color(VGA_LIGHT_CYAN, VGA_BLACK);
    vga_puts("> ");
    vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
}

static void cmd_help(void) {
    vga_puts("DanyaOS Shell v1.1\n\n");
    vga_puts("Commands:\n");
    vga_puts("  help              - show this help\n");
    vga_puts("  clear / cls       - clear screen\n");
    vga_puts("  echo <msg>        - print message\n");
    vga_puts("  uname             - system info\n");
    vga_puts("  mem / free        - memory info\n");
    vga_puts("  uptime            - timer ticks\n");
    vga_puts("  ps                - list processes\n");
    vga_puts("  create <name>     - create process\n");
    vga_puts("  ipc               - test IPC\n");
    vga_puts("  ls                - list files\n");
    vga_puts("  touch <file>      - create file\n");
    vga_puts("  write <f> <data>  - write to file\n");
    vga_puts("  cat <file>        - read file\n");
    vga_puts("  rm <file>         - delete file\n");
    vga_puts("  color <fg> <bg>   - set colors\n");
    vga_puts("  reboot            - reboot (QEMU)\n");
}

static void cmd_clear(void) {
    vga_clear();
}

static void cmd_echo(const char* args) {
    vga_puts(args);
    vga_putchar('\n');
}

static void cmd_uname(void) {
    vga_puts("DanyaOS 1.1.0 (Microkernel)\n");
    vga_puts("Architecture: i386\n");
    vga_puts("Build: GCC cross-compiler\n");
}

static void cmd_mem(void) {
    uint32_t free_mem = pmm_get_free_count() * PAGE_SIZE;
    uint32_t total_mem = pmm_get_total_count() * PAGE_SIZE;
    uint32_t used_mem = total_mem - free_mem;

    vga_printf("Total:  %u KB\n", total_mem / 1024);
    vga_printf("Used:   %u KB\n", used_mem / 1024);
    vga_printf("Free:   %u KB\n", free_mem / 1024);
}

static void cmd_uptime(void) {
    uint32_t ticks = timer_get_ticks();
    uint32_t seconds = ticks / 100;
    vga_printf("Ticks: %u  (%u sec)\n", ticks, seconds);
}

static void cmd_ps(void) {
    vga_puts("  PID  NAME               STATE\n");
    vga_puts("  ---  ----               -----\n");
    int found = 0;
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_t* p = scheduler_get(i + 1);
        if (p) {
            found = 1;
            const char* state = "???";
            switch (p->state) {
                case PROC_READY:   state = "READY"; break;
                case PROC_RUNNING: state = "RUNNING"; break;
                case PROC_BLOCKED: state = "BLOCKED"; break;
                case PROC_ZOMBIE:  state = "ZOMBIE"; break;
                default: break;
            }
            vga_printf("  %-4d %-18s %s\n", p->pid, p->name, state);
        }
    }
    if (!found) vga_puts("  (no processes)\n");
}

static void cmd_touch(const char* name) {
    while (*name == ' ') name++;
    if (*name == '\0') { vga_puts("Usage: touch <filename>\n"); return; }
    if (tmpfs_create(name) == 0)
        vga_printf("Created: %s\n", name);
    else
        vga_puts("Error creating file\n");
}

static void cmd_write_file(const char* args) {
    while (*args == ' ') args++;
    char name[64];
    int i = 0;
    while (*args && *args != ' ' && i < 63) name[i++] = *args++;
    name[i] = '\0';
    while (*args == ' ') args++;
    if (i == 0 || *args == '\0') {
        vga_puts("Usage: write <filename> <data>\n");
        return;
    }
    tmpfs_write(name, args, strlen(args));
    vga_printf("Wrote %d bytes to %s\n", strlen(args), name);
}

static void cmd_cat(const char* name) {
    while (*name == ' ') name++;
    if (*name == '\0') { vga_puts("Usage: cat <filename>\n"); return; }
    char buf[TMPFS_DATA_SIZE + 1];
    int len = tmpfs_read(name, buf, TMPFS_DATA_SIZE);
    if (len < 0) { vga_printf("File not found: %s\n", name); return; }
    buf[len] = '\0';
    vga_puts(buf);
    vga_putchar('\n');
}

static void cmd_rm(const char* name) {
    while (*name == ' ') name++;
    if (*name == '\0') { vga_puts("Usage: rm <filename>\n"); return; }
    if (tmpfs_delete(name) == 0)
        vga_printf("Deleted: %s\n", name);
    else
        vga_printf("Not found: %s\n", name);
}

static void cmd_color(const char* args) {
    static const char* color_names[] = {
        "black", "blue", "green", "cyan", "red", "magenta",
        "brown", "light_grey", "dark_grey", "light_blue",
        "light_green", "light_cyan", "light_red", "light_magenta",
        "yellow", "white"
    };

    while (*args == ' ') args++;
    int fg = -1;
    for (int i = 0; i < 16; i++) {
        if (strncmp(args, color_names[i], strlen(color_names[i])) == 0) {
            fg = i;
            break;
        }
    }

    while (*args && *args != ' ') args++;
    while (*args == ' ') args++;
    int bg = -1;
    for (int i = 0; i < 16; i++) {
        if (strncmp(args, color_names[i], strlen(color_names[i])) == 0) {
            bg = i;
            break;
        }
    }

    if (fg >= 0 && bg >= 0) {
        vga_set_color(fg, bg);
        vga_puts("Color changed!\n");
    } else {
        vga_puts("Usage: color <fg> <bg>\n");
        vga_puts("Colors: black, blue, green, cyan, red, magenta, brown,\n");
        vga_puts("        light_grey, dark_grey, light_blue, light_green,\n");
        vga_puts("        light_cyan, light_red, light_magenta, yellow, white\n");
    }
}

static void cmd_create_process(const char* name) {
    while (*name == ' ') name++;
    if (*name == '\0') { vga_puts("Usage: create <name>\n"); return; }
    process_create(name, (void(*)(void))0);
}

static void cmd_ipc_test(void) {
    ipc_send(1, "Hello from shell!", 17);
    char buf[256];
    int len = ipc_receive(0, buf, 255);
    if (len > 0) {
        buf[len] = '\0';
        vga_printf("IPC received: %s\n", buf);
    } else {
        vga_puts("No IPC messages\n");
    }
}

static void cmd_reboot(void) {
    vga_puts("Rebooting...\n");
    uint8_t good = 0x02;
    while (good & 0x02)
        good = inb(0x64);
    outb(0x64, 0xFE);
    cli();
    hlt();
}

static void process_command(const char* cmd) {
    while (*cmd == ' ') cmd++;
    if (*cmd == '\0') return;

    if (strcmp(cmd, "help") == 0 || strcmp(cmd, "?") == 0) cmd_help();
    else if (strcmp(cmd, "clear") == 0 || strcmp(cmd, "cls") == 0) cmd_clear();
    else if (strncmp(cmd, "echo ", 5) == 0) cmd_echo(cmd + 5);
    else if (strcmp(cmd, "uname") == 0) cmd_uname();
    else if (strcmp(cmd, "mem") == 0 || strcmp(cmd, "free") == 0) cmd_mem();
    else if (strcmp(cmd, "uptime") == 0) cmd_uptime();
    else if (strcmp(cmd, "ps") == 0) cmd_ps();
    else if (strncmp(cmd, "touch ", 6) == 0) cmd_touch(cmd + 6);
    else if (strncmp(cmd, "write ", 6) == 0) cmd_write_file(cmd + 6);
    else if (strncmp(cmd, "cat ", 4) == 0) cmd_cat(cmd + 4);
    else if (strncmp(cmd, "rm ", 3) == 0) cmd_rm(cmd + 3);
    else if (strcmp(cmd, "ls") == 0) tmpfs_list();
    else if (strcmp(cmd, "reboot") == 0) cmd_reboot();
    else if (strncmp(cmd, "color ", 6) == 0) cmd_color(cmd + 6);
    else if (strncmp(cmd, "create ", 7) == 0) cmd_create_process(cmd + 7);
    else if (strcmp(cmd, "ipc") == 0) cmd_ipc_test();
    else {
        vga_set_color(VGA_LIGHT_RED, VGA_BLACK);
        vga_printf("Unknown command: %s\n", cmd);
        vga_set_color(VGA_LIGHT_GREY, VGA_BLACK);
    }
}

void shell_init(void) {
    cmd_len = 0;
    memset(cmd_buf, 0, CMD_BUF_SIZE);
}

void shell_run(void) {
    print_prompt();

    while (1) {
        char c = keyboard_getchar();

        if (c == '\n') {
            cmd_buf[cmd_len] = '\0';
            if (cmd_len > 0) {
                process_command(cmd_buf);
            }
            cmd_len = 0;
            memset(cmd_buf, 0, CMD_BUF_SIZE);
            print_prompt();
        } else if (c == '\b') {
            if (cmd_len > 0) {
                cmd_len--;
                cmd_buf[cmd_len] = '\0';
                vga_putchar('\b');
            }
        } else if (cmd_len < CMD_BUF_SIZE - 1) {
            cmd_buf[cmd_len++] = c;
            vga_putchar(c);
        }
    }
}
