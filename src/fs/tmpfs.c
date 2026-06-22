#include "tmpfs.h"
#include "../memory/heap.h"
#include "../libc/string.h"
#include "../drivers/vga.h"

static tmpfs_file_t files[TMPFS_MAX_FILES];
static int file_count = 0;

void tmpfs_init(void) {
    memset(files, 0, sizeof(files));
    file_count = 0;
    vga_puts("[tmpfs] initialized\n");
}

static tmpfs_file_t* find_file(const char* name) {
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            return &files[i];
        }
    }
    return NULL;
}

int32_t tmpfs_create(const char* name) {
    if (find_file(name)) return 0;
    if (file_count >= TMPFS_MAX_FILES) return -1;

    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (!files[i].used) {
            strncpy(files[i].name, name, TMPFS_NAME_LEN - 1);
            files[i].name[TMPFS_NAME_LEN - 1] = '\0';
            files[i].size = 0;
            files[i].used = true;
            memset(files[i].data, 0, TMPFS_DATA_SIZE);
            file_count++;
            return 0;
        }
    }
    return -1;
}

int32_t tmpfs_write(const char* name, const void* data, uint32_t size) {
    tmpfs_file_t* file = find_file(name);
    if (!file) {
        if (tmpfs_create(name) != 0) return -1;
        file = find_file(name);
    }

    if (size > TMPFS_DATA_SIZE) size = TMPFS_DATA_SIZE;
    memcpy(file->data, data, size);
    file->size = size;
    return (int32_t)size;
}

int32_t tmpfs_read(const char* name, void* buf, uint32_t max_size) {
    tmpfs_file_t* file = find_file(name);
    if (!file) return -1;

    uint32_t len = file->size;
    if (len > max_size) len = max_size;
    memcpy(buf, file->data, len);
    return (int32_t)len;
}

int32_t tmpfs_delete(const char* name) {
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (files[i].used && strcmp(files[i].name, name) == 0) {
            files[i].used = false;
            file_count--;
            return 0;
        }
    }
    return -1;
}

void tmpfs_list(void) {
    if (file_count == 0) {
        vga_puts("  (empty)\n");
        return;
    }
    for (int i = 0; i < TMPFS_MAX_FILES; i++) {
        if (files[i].used) {
            vga_printf("  %-20s %d bytes\n", files[i].name, files[i].size);
        }
    }
}

int32_t tmpfs_exists(const char* name) {
    return find_file(name) != NULL ? 1 : 0;
}
