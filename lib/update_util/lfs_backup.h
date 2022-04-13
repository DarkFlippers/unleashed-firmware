#pragma once

#include <stdbool.h>
#include <storage/storage.h>

#define LFS_BACKUP_DEFAULT_FILENAME "backup.tar"

#ifdef __cplusplus
extern "C" {
#endif

bool lfs_backup_create(Storage* storage, const char* destination);
bool lfs_backup_exists(Storage* storage, const char* source);
bool lfs_backup_unpack(Storage* storage, const char* source);

#ifdef __cplusplus
}
#endif
