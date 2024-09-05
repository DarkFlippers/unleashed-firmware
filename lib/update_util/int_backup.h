#pragma once

#include <stdbool.h>
#include <storage/storage.h>

#define INT_BACKUP_DEFAULT_FILENAME "backup.tar"

#ifdef __cplusplus
extern "C" {
#endif

bool int_backup_create(Storage* storage, const char* destination);
bool int_backup_exists(Storage* storage, const char* source);
bool int_backup_unpack(Storage* storage, const char* source);

#ifdef __cplusplus
}
#endif
