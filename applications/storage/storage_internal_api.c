#include <core/record.h>
#include <m-string.h>
#include "storage.h"
#include <toolbox/tar/tar_archive.h>

FS_Error storage_int_backup(Storage* api, const char* dstname) {
    TarArchive* archive = tar_archive_alloc(api);
    bool success = tar_archive_open(archive, dstname, TAR_OPEN_MODE_WRITE) &&
                   tar_archive_add_dir(archive, STORAGE_INT_PATH_PREFIX, "") &&
                   tar_archive_finalize(archive);
    tar_archive_free(archive);
    return success ? FSE_OK : FSE_INTERNAL;
}

FS_Error storage_int_restore(Storage* api, const char* srcname, Storage_name_converter converter) {
    TarArchive* archive = tar_archive_alloc(api);
    bool success = tar_archive_open(archive, srcname, TAR_OPEN_MODE_READ) &&
                   tar_archive_unpack_to(archive, STORAGE_INT_PATH_PREFIX, converter);
    tar_archive_free(archive);
    return success ? FSE_OK : FSE_INTERNAL;
}
