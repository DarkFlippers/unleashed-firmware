#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct TarArchive TarArchive;

typedef struct Storage Storage;

/** Tar archive open mode 
 */
typedef enum {
    TarOpenModeRead = 'r',
    TarOpenModeWrite = 'w',
    /* read-only heatshrink compressed tar */
    TarOpenModeReadHeatshrink = 'h',
} TarOpenMode;

/** Get expected open mode for archive at the path.
 * Used for automatic mode detection based on the file extension.
 *
 * @param[in]   path          Path to the archive
 *
 * @return open mode from TarOpenMode enum
 */
TarOpenMode tar_archive_get_mode_for_path(const char* path);

/** Tar archive constructor
 *
 * @param storage             Storage API pointer
 *
 * @return allocated object
 */
TarArchive* tar_archive_alloc(Storage* storage);

/** Open tar archive
 *
 * @param       archive       Tar archive object
 * @param[in]   path          Path to the tar archive
 * @param       mode          Open mode
 *
 * @return true if successful
 */
bool tar_archive_open(TarArchive* archive, const char* path, TarOpenMode mode);

/** Tar archive destructor
 *
 * @param archive Tar archive object
 */
void tar_archive_free(TarArchive* archive);

typedef void (*TarArchiveNameConverter)(FuriString*);

/* High-level API  - assumes archive is open */

/** Unpack tar archive to destination
 *
 * @param       archive       Tar archive object. Must be opened in read mode
 * @param[in]   destination   Destination path
 * @param       converter     Storage name converter
 *
 * @return true if successful
 */
bool tar_archive_unpack_to(
    TarArchive* archive,
    const char* destination,
    TarArchiveNameConverter converter);

/** Add file to tar archive
 *
 * @param       archive       Tar archive object. Must be opened in write mode
 * @param[in]   fs_file_path  Path to the file on the filesystem
 * @param[in]   archive_fname Name of the file in the archive
 * @param       file_size     Size of the file
 *
 * @return true if successful
 */
bool tar_archive_add_file(
    TarArchive* archive,
    const char* fs_file_path,
    const char* archive_fname,
    const int32_t file_size);

/** Add directory to tar archive
 *
 * @param       archive       Tar archive object. Must be opened in write mode
 * @param       fs_full_path  Path to the directory on the filesystem
 * @param       path_prefix   Prefix to add to the directory name in the archive
 *
 * @return true if successful
 */
bool tar_archive_add_dir(TarArchive* archive, const char* fs_full_path, const char* path_prefix);

/** Get number of entries in the archive
 *
 * @param archive Tar archive object
 *
 * @return number of entries. -1 on error
 */
int32_t tar_archive_get_entries_count(TarArchive* archive);

/** Get read progress
 *
 * @param       archive Tar archive object. Must be opened in read mode
 * @param[in]   processed Number of processed entries
 * @param[in]   total Total number of entries
 *
 * @return true if successful
 */
bool tar_archive_get_read_progress(TarArchive* archive, int32_t* processed, int32_t* total);

/** Unpack single file from tar archive
 *
 * @param       archive       Tar archive object. Must be opened in read mode
 * @param[in]   archive_fname Name of the file in the archive
 * @param[in]   destination   Destination path
 *
 * @return true if successful
 */
bool tar_archive_unpack_file(
    TarArchive* archive,
    const char* archive_fname,
    const char* destination);

/** Optional per-entry callback on unpacking
 * @param       name          Name of the file or directory
 * @param       is_directory  True if the entry is a directory
 * @param[in]   context       User context
 * @return true to process the entry, false to skip
 */
typedef bool (*tar_unpack_file_cb)(const char* name, bool is_directory, void* context);

/** Set per-entry callback on unpacking
 * @param       archive       Tar archive object
 * @param       callback      Callback function
 * @param[in]   context       User context
 */
void tar_archive_set_file_callback(TarArchive* archive, tar_unpack_file_cb callback, void* context);

/* Low-level API */

/** Add tar archive directory header
 *
 * @param       archive       Tar archive object. Must be opened in write mode
 * @param[in]   dirpath       Path to the directory
 *
 * @return true if successful
 */
bool tar_archive_dir_add_element(TarArchive* archive, const char* dirpath);

/** Add tar archive file header
 *
 * @param       archive       Tar archive object. Must be opened in write mode
 * @param[in]   path          Path to the file
 * @param       data_len      Size of the file
 *
 * @return true if successful
 */
bool tar_archive_file_add_header(TarArchive* archive, const char* path, const int32_t data_len);

/** Add tar archive file data block
 *
 * @param       archive       Tar archive object. Must be opened in write mode
 * @param[in]   data_block    Data block
 * @param       block_len     Size of the data block
 *
 * @return true if successful
 */
bool tar_archive_file_add_data_block(
    TarArchive* archive,
    const uint8_t* data_block,
    const int32_t block_len);

/** Finalize tar archive file
 *
 * @param archive       Tar archive object. Must be opened in write mode
 *
 * @return true if successful
 */
bool tar_archive_file_finalize(TarArchive* archive);

/** Store data in tar archive
 *
 * @param       archive       Tar archive object. Must be opened in write mode
 * @param[in]   path          Path to the file
 * @param[in]   data          Data to store
 * @param       data_len      Size of the data
 *
 * @return true if successful
 */
bool tar_archive_store_data(
    TarArchive* archive,
    const char* path,
    const uint8_t* data,
    const int32_t data_len);

/** Finalize tar archive
 *
 * @param archive       Tar archive object. Must be opened in write mode
 *
 * @return true if successful
 */
bool tar_archive_finalize(TarArchive* archive);

#ifdef __cplusplus
}
#endif
