#include <core/record.h>
#include "storage.h"
#include <toolbox/tar/tar_archive.h>

FS_Error storage_int_backup(Storage* storage, const char* dstname) {
    furi_check(storage);

    TarArchive* archive = tar_archive_alloc(storage);
    bool success = tar_archive_open(archive, dstname, TarOpenModeWrite) &&
                   tar_archive_add_dir(archive, STORAGE_INT_PATH_PREFIX, "") &&
                   tar_archive_finalize(archive);
    tar_archive_free(archive);
    return success ? FSE_OK : FSE_INTERNAL;
}

FS_Error
    storage_int_restore(Storage* storage, const char* srcname, StorageNameConverter converter) {
    furi_check(storage);

    TarArchive* archive = tar_archive_alloc(storage);
    bool success = tar_archive_open(archive, srcname, TarOpenModeRead) &&
                   tar_archive_unpack_to(archive, STORAGE_INT_PATH_PREFIX, converter);
    tar_archive_free(archive);
    return success ? FSE_OK : FSE_INTERNAL;
}
