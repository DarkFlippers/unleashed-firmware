#include "lfs_backup.h"

#include <toolbox/tar/tar_archive.h>

#define LFS_BACKUP_DEFAULT_LOCATION "/ext/" LFS_BACKUP_DEFAULT_FILENAME

bool lfs_backup_create(Storage* storage, const char* destination) {
    const char* final_destination =
        destination && strlen(destination) ? destination : LFS_BACKUP_DEFAULT_LOCATION;
    return storage_int_backup(storage, final_destination) == FSE_OK;
}

bool lfs_backup_exists(Storage* storage, const char* source) {
    const char* final_source = source && strlen(source) ? source : LFS_BACKUP_DEFAULT_LOCATION;
    FileInfo fi;
    return storage_common_stat(storage, final_source, &fi) == FSE_OK;
}

bool lfs_backup_unpack(Storage* storage, const char* source) {
    const char* final_source = source && strlen(source) ? source : LFS_BACKUP_DEFAULT_LOCATION;
    return storage_int_restore(storage, final_source) == FSE_OK;
}
