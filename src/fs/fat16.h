#ifndef DANYA_FAT16_H
#define DANYA_FAT16_H

#include "../include/types.h"

#define FAT16_MAX_FILES 64
#define FAT16_NAME_LEN  11
#define FAT16_PATH_LEN  128

typedef struct {
    uint8_t  boot_jmp[3];
    uint8_t  oem_name[8];
    uint16_t bytes_per_sector;
    uint8_t  sectors_per_cluster;
    uint16_t reserved_sectors;
    uint8_t  num_fats;
    uint16_t root_entries;
    uint16_t total_sectors_16;
    uint8_t  media_type;
    uint16_t fat_size_16;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t hidden_sectors;
    uint32_t total_sectors_32;
    uint8_t  drive_number;
    uint8_t  reserved;
    uint8_t  boot_signature;
    uint32_t volume_serial;
    uint8_t  volume_label[11];
    uint8_t  filesystem_type[8];
} __attribute__((packed)) fat16_bpb_t;

typedef struct {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_reserved;
    uint8_t  create_time_tenth;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t first_cluster_high;
    uint16_t modify_time;
    uint16_t modify_date;
    uint16_t first_cluster;
    uint32_t file_size;
} __attribute__((packed)) fat16_dir_entry_t;

typedef struct {
    char     name[13];
    uint32_t size;
    uint16_t first_cluster;
    uint8_t  attr;
    uint8_t  used;
} fat16_file_t;

typedef struct {
    fat16_bpb_t bpb;
    uint32_t fat_start;
    uint32_t root_start;
    uint32_t data_start;
    uint32_t total_clusters;
    uint8_t  mounted;
    fat16_file_t files[FAT16_MAX_FILES];
    int      file_count;
} fat16_fs_t;

int  fat16_mount(void);
int  fat16_read_file(const char* name, void* buf, uint32_t max_size);
int  fat16_write_file(const char* name, const void* data, uint32_t size);
int  fat16_delete_file(const char* name);
int  fat16_create_file(const char* name);
void fat16_list_files(void);
int  fat16_file_exists(const char* name);
fat16_fs_t* fat16_get_fs(void);

#endif
