#include "ipc.h"
#include "scheduler.h"
#include "../memory/heap.h"
#include "../libc/string.h"
#include "../drivers/vga.h"

static ipc_message_t message_queue[IPC_MAX_MSGS];
static uint32_t queue_head = 0;
static uint32_t queue_tail = 0;

void ipc_init(void) {
    memset(message_queue, 0, sizeof(message_queue));
    queue_head = 0;
    queue_tail = 0;
}

int32_t ipc_send(pid_t to, const char* data, uint32_t length) {
    if (length > IPC_MSG_SIZE) length = IPC_MSG_SIZE;

    uint32_t next = (queue_tail + 1) % IPC_MAX_MSGS;
    if (next == queue_head) {
        vga_puts("[ipc] message queue full\n");
        return -1;
    }

    process_t* sender = scheduler_current();
    process_t* receiver = scheduler_get(to);
    if (!receiver) {
        vga_printf("[ipc] pid %d not found\n", to);
        return -1;
    }

    ipc_message_t* msg = &message_queue[queue_tail];
    msg->sender = sender ? sender->pid : 0;
    msg->receiver = to;
    memcpy(msg->data, data, length);
    msg->length = length;
    msg->used = true;

    queue_tail = next;
    return (int32_t)length;
}

int32_t ipc_receive(pid_t from, char* buf, uint32_t max_len) {
    uint32_t current = queue_head;
    while (current != queue_tail) {
        ipc_message_t* msg = &message_queue[current];
        if (msg->used && (from == 0 || msg->sender == from)) {
            uint32_t len = msg->length;
            if (len > max_len) len = max_len;
            memcpy(buf, msg->data, len);
            msg->used = false;
            queue_head = (current + 1) % IPC_MAX_MSGS;
            return (int32_t)len;
        }
        current = (current + 1) % IPC_MAX_MSGS;
    }
    return 0;
}
