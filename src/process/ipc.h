#ifndef DANYA_IPC_H
#define DANYA_IPC_H

#include "types.h"

#define IPC_MSG_SIZE 256
#define IPC_MAX_MSGS 64

typedef struct {
    pid_t sender;
    pid_t receiver;
    char  data[IPC_MSG_SIZE];
    uint32_t length;
    bool  used;
} ipc_message_t;

void    ipc_init(void);
int32_t ipc_send(pid_t to, const char* data, uint32_t length);
int32_t ipc_receive(pid_t from, char* buf, uint32_t max_len);

#endif
