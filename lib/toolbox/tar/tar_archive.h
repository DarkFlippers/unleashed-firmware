#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <m-string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TarArchive TarArchive;

typedef struct Storage Storage;

typedef enum {
    TAR_OPEN_MODE_READ = 'r',
    TAR_OPEN_MODE_WRITE = 'w',
    TAR_OPEN_MODE_STDOUT = 's' /* to be implemented */
} TarOpenMode;

TarArchive* tar_archive_alloc(Storage* storage);

bool tar_archive_open(TarArchive* archive, const char* path, TarOpenMode mode);

void tar_archive_free(TarArchive* archive);

/* High-level API  - assumes archive is open */
bool tar_archive_unpack_to(
    TarArchive* archive,
    const char* destination,
    Storage_name_converter converter);

bool tar_archive_add_file(
    TarArchive* archive,
    const char* fs_file_path,
    const char* archive_fname,
    const int32_t file_size);

bool tar_archive_add_dir(TarArchive* archive, const char* fs_full_path, const char* path_prefix);

int32_t tar_archive_get_entries_count(TarArchive* archive);

bool tar_archive_unpack_file(
    TarArchive* archive,
    const char* archive_fname,
    const char* destination);

/* Optional per-entry callback on unpacking - return false to skip entry */
typedef bool (*tar_unpack_file_cb)(const char* name, bool is_directory, void* context);

void tar_archive_set_file_callback(TarArchive* archive, tar_unpack_file_cb callback, void* context);

/* Low-level API */
bool tar_archive_dir_add_element(TarArchive* archive, const char* dirpath);

bool tar_archive_file_add_header(TarArchive* archive, const char* path, const int32_t data_len);

bool tar_archive_file_add_data_block(
    TarArchive* archive,
    const uint8_t* data_block,
    const int32_t block_len);

bool tar_archive_file_finalize(TarArchive* archive);

bool tar_archive_store_data(
    TarArchive* archive,
    const char* path,
    const uint8_t* data,
    const int32_t data_len);

bool tar_archive_finalize(TarArchive* archive);

#ifdef __cplusplus
}
#endif
