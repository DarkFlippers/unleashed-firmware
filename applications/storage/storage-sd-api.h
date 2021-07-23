#pragma once
#include <furi.h>
#include "filesystem-api-defines.h"
#include <fatfs.h>
#include "storage-glue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SD_LABEL_LENGTH 34

typedef enum {
    FST_FAT12 = FS_FAT12,
    FST_FAT16 = FS_FAT16,
    FST_FAT32 = FS_FAT32,
    FST_EXFAT = FS_EXFAT,
} SDFsType;

typedef struct {
    SDFsType fs_type;
    uint32_t kb_total;
    uint32_t kb_free;
    uint16_t cluster_size;
    uint16_t sector_size;
    char label[SD_LABEL_LENGTH];
    FS_Error error;
} SDInfo;

const char* sd_api_get_fs_type_text(SDFsType fs_type);

#ifdef __cplusplus
}
#endif