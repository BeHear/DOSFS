#ifndef DANYA_SYSCALL_H
#define DANYA_SYSCALL_H

#include "../kernel/idt.h"

#define SYSCALL_WRITE    1
#define SYSCALL_READ     2
#define SYSCALL_EXIT     3
#define SYSCALL_FORK     4
#define SYSCALL_EXEC     5
#define SYSCALL_GETPID   6
#define SYSCALL_IPC_SEND 7
#define SYSCALL_IPC_RECV 8
#define SYSCALL_SLEEP    9
#define SYSCALL_MEMINFO  10

void syscall_init(void);
void syscall_handler(stack_state_t* state);

#endif
