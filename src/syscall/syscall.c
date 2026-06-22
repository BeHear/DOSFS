#include "syscall.h"
#include "../kernel/idt.h"
#include "../process/scheduler.h"
#include "../process/ipc.h"
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"
#include "../memory/pmm.h"
#include "../libc/string.h"

static void sys_write(stack_state_t* state) {
    const char* buf = (const char*)state->ebx;
    uint32_t len = state->ecx;
    for (uint32_t i = 0; i < len; i++) {
        vga_putchar(buf[i]);
    }
}

static void sys_read(stack_state_t* state) {
    char* buf = (char*)state->ebx;
    uint32_t max_len = state->ecx;
    uint32_t i = 0;

    while (i < max_len - 1) {
        char c = keyboard_getchar();
        if (c == '\n') {
            vga_putchar('\n');
            break;
        }
        if (c == '\b') {
            if (i > 0) {
                i--;
                vga_putchar('\b');
            }
            continue;
        }
        buf[i++] = c;
        vga_putchar(c);
    }
    buf[i] = '\0';
    state->eax = i;
}

static void sys_exit(stack_state_t* state) {
    process_exit((int)state->ebx);
}

static void sys_getpid(stack_state_t* state) {
    process_t* proc = scheduler_current();
    state->eax = proc ? proc->pid : -1;
}

static void sys_ipc_send(stack_state_t* state) {
    pid_t to = (pid_t)state->ebx;
    const char* data = (const char*)state->ecx;
    uint32_t len = state->edx;
    state->eax = ipc_send(to, data, len);
}

static void sys_ipc_recv(stack_state_t* state) {
    pid_t from = (pid_t)state->ebx;
    char* buf = (char*)state->ecx;
    uint32_t max_len = state->edx;
    state->eax = ipc_receive(from, buf, max_len);
}

static void sys_sleep(stack_state_t* state) {
    UNUSED(state);
    process_yield();
}

static void sys_meminfo(stack_state_t* state) {
    state->eax = pmm_get_free_count() * PAGE_SIZE;
    state->ebx = pmm_get_total_count() * PAGE_SIZE;
}

void syscall_handler(stack_state_t* state) {
    switch (state->eax) {
        case SYSCALL_WRITE:    sys_write(state); break;
        case SYSCALL_READ:     sys_read(state); break;
        case SYSCALL_EXIT:     sys_exit(state); break;
        case SYSCALL_GETPID:   sys_getpid(state); break;
        case SYSCALL_IPC_SEND: sys_ipc_send(state); break;
        case SYSCALL_IPC_RECV: sys_ipc_recv(state); break;
        case SYSCALL_SLEEP:    sys_sleep(state); break;
        case SYSCALL_MEMINFO:  sys_meminfo(state); break;
        default:
            vga_printf("[syscall] unknown syscall %d\n", state->eax);
            state->eax = -1;
            break;
    }
}

void syscall_init(void) {
    idt_register_handler(128, syscall_handler);
}
