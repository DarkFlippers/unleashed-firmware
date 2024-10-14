/**
 * @file storage.h
 * @brief APIs for working with storages, directories and files.
 */
#pragma once

#include <stdint.h>
#include "filesystem_api_defines.h"
#include "storage_sd_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STORAGE_INT_PATH_PREFIX        "/int"
#define STORAGE_EXT_PATH_PREFIX        "/ext"
#define STORAGE_ANY_PATH_PREFIX        "/any"
#define STORAGE_APP_DATA_PATH_PREFIX   "/data"
#define STORAGE_APP_ASSETS_PATH_PREFIX "/assets"

#define INT_PATH(path)        STORAGE_INT_PATH_PREFIX "/" path
#define EXT_PATH(path)        STORAGE_EXT_PATH_PREFIX "/" path
#define ANY_PATH(path)        STORAGE_ANY_PATH_PREFIX "/" path
#define APP_DATA_PATH(path)   STORAGE_APP_DATA_PATH_PREFIX "/" path
#define APP_ASSETS_PATH(path) STORAGE_APP_ASSETS_PATH_PREFIX "/" path

#define RECORD_STORAGE "storage"

typedef struct Storage Storage;

/**
 * @brief Allocate and initialize a file instance.
 *
 * @param storage pointer to a storage API instance.
 * @return pointer to the created instance.
 */
File* storage_file_alloc(Storage* storage);

/**
 * @brief Free the file instance.
 *
 * If the file was open, calling this function will close it automatically.
 * @param file pointer to the file instance to be freed.
 */
void storage_file_free(File* file);

/**
 * @brief Enumeration of events emitted by the storage through the PubSub system.
 */
typedef enum {
    StorageEventTypeCardMount, /**< SD card was mounted. */
    StorageEventTypeCardUnmount, /**< SD card was unmounted. */
    StorageEventTypeCardMountError, /**< An error occurred during mounting of an SD card. */
    StorageEventTypeFileClose, /**< A file was closed. */
    StorageEventTypeDirClose, /**< A directory was closed. */
} StorageEventType;

/**
 * @brief Storage event (passed to the PubSub callback).
 */
typedef struct {
    StorageEventType type; /**< Type of the event. */
} StorageEvent;

/**
 * @brief Get the storage pubsub instance.
 *
 * Storage will send StorageEvent messages.
 *
 * @param storage pointer to a storage API instance.
 * @return pointer to the pubsub instance.
 */
FuriPubSub* storage_get_pubsub(Storage* storage);

/******************* File Functions *******************/

/**
 * @brief Open an existing file or create a new one.
 *
 * @warning The calling code MUST call storage_file_close() even if the open operation had failed.
 *
 * @param file pointer to the file instance to be opened.
 * @param path pointer to a zero-terminated string containing the path to the file to be opened.
 * @param access_mode access mode from FS_AccessMode.
 * @param open_mode open mode from FS_OpenMode 
 * @return true if the file was successfully opened, false otherwise.
 */
bool storage_file_open(
    File* file,
    const char* path,
    FS_AccessMode access_mode,
    FS_OpenMode open_mode);

/**
 * @brief Close the file.
 *
 * @param file pointer to the file instance to be closed.
 * @return true if the file was successfully closed, false otherwise.
 */
bool storage_file_close(File* file);

/**
 * @brief Check whether the file is open.
 *
 * @param file pointer to the file instance in question.
 * @return true if the file is open, false otherwise.
 */
bool storage_file_is_open(File* file);

/**
 * @brief Check whether a file instance represents a directory.
 *
 * @param file pointer to the file instance in question.
 * @return true if the file instance represents a directory, false otherwise.
 */
bool storage_file_is_dir(File* file);

/**
 * @brief Read bytes from a file into a buffer.
 *
 * @param file pointer to the file instance to read from.
 * @param buff pointer to the buffer to be filled with read data.
 * @param bytes_to_read number of bytes to read. Must be less than or equal to the size of the buffer.
 * @return actual number of bytes read (may be fewer than requested).
 */
size_t storage_file_read(File* file, void* buff, size_t bytes_to_read);

/**
 * @brief Write bytes from a buffer to a file.
 *
 * @param file pointer to the file instance to write into.
 * @param buff pointer to the buffer containing the data to be written.
 * @param bytes_to_write number of bytes to write. Must be less than or equal to the size of the buffer.
 * @return actual number of bytes written (may be fewer than requested).
 */
size_t storage_file_write(File* file, const void* buff, size_t bytes_to_write);

/**
 * @brief Change the current access position in a file.
 *
 * @param file pointer to the file instance in question.
 * @param offset access position offset (meaning depends on from_start parameter).
 * @param from_start if true, set the access position relative to the file start, otherwise relative to the current position.
 * @return success flag
 */
bool storage_file_seek(File* file, uint32_t offset, bool from_start);

/**
 * @brief Get the current access position.
 *
 * @param file pointer to the file instance in question.
 * @return current access position.
 */
uint64_t storage_file_tell(File* file);

/**
 * @brief Truncate the file size to the current access position.
 *
 * @param file pointer to the file instance to be truncated.
 * @return true if the file was successfully truncated, false otherwise.
 */
bool storage_file_truncate(File* file);

/**
 * @brief Get the file size.
 *
 * @param file pointer to the file instance in question.
 * @return size of the file, in bytes.
 */
uint64_t storage_file_size(File* file);

/**
 * @brief Synchronise the file cache with the actual storage.
 *
 * @param file pointer to the file instance in question.
 * @return true if the file was successfully synchronised, false otherwise.
 */
bool storage_file_sync(File* file);

/**
 * @brief Check whether the current access position is at the end of the file.
 *
 * @param file pointer to a file instance in question.
 * @return bool true if the current access position is at the end of the file, false otherwise.
 */
bool storage_file_eof(File* file);

/**
 * @brief Check whether a file exists.
 * 
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path to the file in question.
 * @return true if the file exists, false otherwise.
 */
bool storage_file_exists(Storage* storage, const char* path);

/**
 * @brief Copy data from a source file to the destination file.
 *
 * Both files must be opened prior to calling this function.
 *
 * The requested amount of bytes will be copied from the current access position
 * in the source file to the current access position in the destination file.
 * 
 * @param source pointer to a source file instance.
 * @param destination pointer to a destination file instance.
 * @param size data size to be copied, in bytes.
 * @return true if the data was successfully copied, false otherwise.
 */
bool storage_file_copy_to_file(File* source, File* destination, size_t size);

/******************* Directory Functions *******************/

/**
 * @brief Open a directory.
 *
 * Opening a directory is necessary to be able to read its contents with storage_dir_read().
 *
 * @warning The calling code MUST call storage_dir_close() even if the open operation had failed.
 *
 * @param file pointer to a file instance representing the directory in question.
 * @param path pointer to a zero-terminated string containing the path of the directory in question.
 * @return true if the directory was successfully opened, false otherwise.
 */
bool storage_dir_open(File* file, const char* path);

/**
 * @brief Close the directory.
 *
 * @param file pointer to a file instance representing the directory in question.
 * @return true if the directory was successfully closed, false otherwise.
 */
bool storage_dir_close(File* file);

/**
 * @brief Get the next item in the directory.
 *
 * If the next object does not exist, this function returns false as well
 * and sets the file error id to FSE_NOT_EXIST.
 *
 * @param file pointer to a file instance representing the directory in question.
 * @param fileinfo pointer to the FileInfo structure to contain the info (may be NULL).
 * @param name pointer to the buffer to contain the name (may be NULL).
 * @param name_length maximum capacity of the name buffer, in bytes.
 * @return true if the next item was successfully read, false otherwise.
 */
bool storage_dir_read(File* file, FileInfo* fileinfo, char* name, uint16_t name_length);

/**
 * @brief Change the access position to first item in the directory.
 *
 * @param file pointer to a file instance representing the directory in question.
 * @return true if the access position was successfully changed, false otherwise.
 */
bool storage_dir_rewind(File* file);

/**
 * @brief Check whether a directory exists.
 * 
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path of the directory in question.
 * @return true if the directory exists, false otherwise.
 */
bool storage_dir_exists(Storage* storage, const char* path);

/******************* Common Functions *******************/

/**
 * @brief Get the last access time in UNIX format.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path of the item in question.
 * @param timestamp pointer to a value to contain the timestamp.
 * @return FSE_OK if the timestamp has been successfully received, any other error code on failure.
 */
FS_Error storage_common_timestamp(Storage* storage, const char* path, uint32_t* timestamp);

/**
 * @brief Get information about a file or a directory.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path of the item in question.
 * @param fileinfo pointer to the FileInfo structure to contain the info (may be NULL).
 * @return FSE_OK if the info has been successfully received, any other error code on failure.
 */
FS_Error storage_common_stat(Storage* storage, const char* path, FileInfo* fileinfo);

/**
 * @brief Remove a file or a directory.
 *
 * The directory must be empty.
 * The file or the directory must NOT be open.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path of the item to be removed.
 * @return FSE_OK if the file or directory has been successfully removed, any other error code on failure.
 */
FS_Error storage_common_remove(Storage* storage, const char* path);

/**
 * @brief Rename a file or a directory.
 *
 * The file or the directory must NOT be open.
 * Will overwrite the destination file if it already exists.
 *
 * Renaming a regular file to itself does nothing and always succeeds.
 * Renaming a directory to itself or to a subdirectory of itself always fails.
 *
 * @param storage pointer to a storage API instance.
 * @param old_path pointer to a zero-terminated string containing the source path.
 * @param new_path pointer to a zero-terminated string containing the destination path.
 * @return FSE_OK if the file or directory has been successfully renamed, any other error code on failure.
 */
FS_Error storage_common_rename(Storage* storage, const char* old_path, const char* new_path);

/**
 * @brief Copy the file to a new location.
 *
 * The file must NOT be open at the time of calling this function.
 *
 * @param storage pointer to a storage API instance.
 * @param old_path pointer to a zero-terminated string containing the source path.
 * @param new_path pointer to a zero-terminated string containing the destination path.
 * @return FSE_OK if the file has been successfully copied, any other error code on failure.
 */
FS_Error storage_common_copy(Storage* storage, const char* old_path, const char* new_path);

/**
 * @brief Copy the contents of one directory into another and rename all conflicting files.
 *
 * @param storage pointer to a storage API instance.
 * @param old_path pointer to a zero-terminated string containing the source path.
 * @param new_path pointer to a zero-terminated string containing the destination path.
 * @return FSE_OK if the directories have been successfully merged, any other error code on failure.
 */
FS_Error storage_common_merge(Storage* storage, const char* old_path, const char* new_path);

/**
 * @brief Create a directory.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the directory path.
 * @return FSE_OK if the directory has been successfully created, any other error code on failure.
 */
FS_Error storage_common_mkdir(Storage* storage, const char* path);

/**
 * @brief Get the general information about the storage.
 *
 * @param storage pointer to a storage API instance.
 * @param fs_path pointer to a zero-terminated string containing the path to the storage question.
 * @param total_space pointer to the value to contain the total capacity, in bytes.
 * @param free_space pointer to the value to contain the available space, in bytes.
 * @return FSE_OK if the information has been successfully received, any other error code on failure.
 */
FS_Error storage_common_fs_info(
    Storage* storage,
    const char* fs_path,
    uint64_t* total_space,
    uint64_t* free_space);

/**
 * @brief Parse aliases in a path and replace them with the real path.
 *
 * Necessary special directories will be created automatically if they did not exist.
 * 
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path in question.
 */
void storage_common_resolve_path_and_ensure_app_directory(Storage* storage, FuriString* path);

/**
 * @brief Move the contents of source folder to destination one and rename all conflicting files.
 *
 * Source folder will be deleted if the migration was successful.
 * 
 * @param storage pointer to a storage API instance.
 * @param source pointer to a zero-terminated string containing the source path.
 * @param dest pointer to a zero-terminated string containing the destination path.
 * @return FSE_OK if the migration was successfully completed, any other error code on failure.
 */
FS_Error storage_common_migrate(Storage* storage, const char* source, const char* dest);

/**
 * @brief Check whether a file or a directory exists.
 * 
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the path in question.
 * @return true if a file or a directory exists, false otherwise.
 */
bool storage_common_exists(Storage* storage, const char* path);

/**
 * @brief Check whether two paths are equivalent.
 *
 * This function will resolve aliases and apply filesystem-specific
 * rules to determine whether the two given paths are equivalent.
 *
 * Examples:
 * - /int/text and /ext/test -> false (Different storages),
 * - /int/Test and /int/test -> false (Case-sensitive storage),
 * - /ext/Test and /ext/test -> true (Case-insensitive storage).
 *
 * @param storage pointer to a storage API instance.
 * @param path1 pointer to a zero-terminated string containing the first path.
 * @param path2 pointer to a zero-terminated string containing the second path.
 * @return true if paths are equivalent, false otherwise.
 */
bool storage_common_equivalent_path(Storage* storage, const char* path1, const char* path2);

/**
 * @brief Check whether a path is a subpath of another path.
 * 
 * This function respects storage-defined equivalence rules
 * (see `storage_common_equivalent_path`).
 * 
 * @param storage pointer to a storage API instance.
 * @param parent pointer to a zero-terminated string containing the parent path.
 * @param child pointer to a zero-terminated string containing the child path.
 * @return true if `child` is a subpath of `parent`, or if `child` is equivalent
 *         to `parent`; false otherwise.
 */
bool storage_common_is_subdir(Storage* storage, const char* parent, const char* child);

/******************* Error Functions *******************/

/**
 * @brief Get the textual description of a numeric error identifier.
 *
 * @param error_id numeric identifier of the error in question.
 * @return pointer to a statically allocated zero-terminated string containing the respective error text.
 */
const char* storage_error_get_desc(FS_Error error_id);

/**
 * @brief Get the numeric error identifier from a file instance.
 *
 * @warning It is not possible to get the error identifier after the file has been closed.
 *
 * @param file pointer to the file instance in question (must NOT be NULL).
 * @return numeric identifier of the last error associated with the file instance.
 */
FS_Error storage_file_get_error(File* file);

/**
 * @brief Get the internal (storage-specific) numeric error identifier from a file instance.
 *
 * @warning It is not possible to get the internal error identifier after the file has been closed.
 *
 * @param file pointer to the file instance in question (must NOT be NULL).
 * @return numeric identifier of the last internal error associated with the file instance.
 */
int32_t storage_file_get_internal_error(File* file);

/**
 * @brief Get the textual description of a the last error associated with a file instance.
 *
 * @warning It is not possible to get the error text after the file has been closed.
 *
 * @param file pointer to the file instance in question (must NOT be NULL).
 * @return pointer to a statically allocated zero-terminated string containing the respective error text.
 */
const char* storage_file_get_error_desc(File* file);

/******************* SD Card Functions *******************/

/**
 * @brief Format the SD Card.
 *
 * @param storage pointer to a storage API instance.
 * @return FSE_OK if the card was successfully formatted, any other error code on failure.
 */
FS_Error storage_sd_format(Storage* storage);

/**
 * @brief Unmount the SD card.
 *
 * These return values have special meaning:
 * - FSE_NOT_READY if the SD card is not mounted.
 * - FSE_DENIED if there are open files on the SD card.
 *
 * @param storage pointer to a storage API instance.
 * @return FSE_OK if the card was successfully formatted, any other error code on failure.
 */
FS_Error storage_sd_unmount(Storage* storage);

/**
 * @brief Mount the SD card.
 *
 * @param storage pointer to a storage API instance.
 * @return FSE_OK if the card was successfully mounted, any other error code on failure.
 */
FS_Error storage_sd_mount(Storage* storage);

/**
 * @brief Get SD card information.
 *
 * @param storage pointer to a storage API instance.
 * @param info pointer to the info object to contain the requested information.
 * @return FSE_OK if the info was successfully received, any other error code on failure.
 */
FS_Error storage_sd_info(Storage* storage, SDInfo* info);

/**
 * @brief Get SD card status.
 *
 * @param storage pointer to a storage API instance.
 * @return storage status in the form of a numeric error identifier.
 */
FS_Error storage_sd_status(Storage* storage);

/************ Internal Storage Backup/Restore ************/

typedef void (*StorageNameConverter)(FuriString*);

/**
 * @brief Back up the internal storage contents to a *.tar archive.
 *
 * @param storage pointer to a storage API instance.
 * @param dstname pointer to a zero-terminated string containing the archive file path.
 * @return FSE_OK if the storage was successfully backed up, any other error code on failure.
 */
FS_Error storage_int_backup(Storage* storage, const char* dstname);

/**
 * @brief Restore the internal storage contents from a *.tar archive.
 *
 * @param storage pointer to a storage API instance.
 * @param dstname pointer to a zero-terminated string containing the archive file path.
 * @param converter pointer to a filename conversion function (may be NULL).
 * @return FSE_OK if the storage was successfully restored, any other error code on failure.
 */
FS_Error
    storage_int_restore(Storage* storage, const char* dstname, StorageNameConverter converter);

/***************** Simplified Functions ******************/

/**
 * @brief Remove a file or a directory.
 *
 * The following conditions must be met:
 * - the directory must be empty.
 * - the file or the directory must NOT be open.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the item path.
 * @return true on success or if the item does not exist, false otherwise.
 */
bool storage_simply_remove(Storage* storage, const char* path);

/**
 * @brief Recursively remove a file or a directory.
 *
 * Unlike storage_simply_remove(), the directory does not need to be empty.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the item path.
 * @return true on success or if the item does not exist, false otherwise.
 */
bool storage_simply_remove_recursive(Storage* storage, const char* path);

/**
 * @brief Create a directory.
 *
 * @param storage pointer to a storage API instance.
 * @param path pointer to a zero-terminated string containing the directory path.
 * @return true on success or if directory does already exist, false otherwise.
 */
bool storage_simply_mkdir(Storage* storage, const char* path);

/**
 * @brief Get the next free filename in a directory.
 *
 * Usage example:
 * ```c
 * FuriString* file_name = furi_string_alloc();
 * Storage* storage = furi_record_open(RECORD_STORAGE);
 *
 * storage_get_next_filename(storage,
 *     "/ext/test",
 *     "cookies",
 *     ".yum",
 *     20);
 *
 * furi_record_close(RECORD_STORAGE);
 *
 * use_file_name(file_name);
 *
 * furi_string_free(file_name);
 * ```
 * Possible file_name values after calling storage_get_next_filename():
 * "cookies", "cookies1", "cookies2", ... etc depending on whether any of
 * these files have already existed in the directory.
 *
 * @note If the resulting next file name length is greater than set by the max_len
 * parameter, the original filename will be returned instead.
 * 
 * @param storage pointer to a storage API instance.
 * @param dirname pointer to a zero-terminated string containing the directory path.
 * @param filename pointer to a zero-terminated string containing the file name.
 * @param fileextension pointer to a zero-terminated string containing the file extension.
 * @param nextfilename pointer to a dynamic string containing the resulting file name.
 * @param max_len maximum length of the new name.
 */
void storage_get_next_filename(
    Storage* storage,
    const char* dirname,
    const char* filename,
    const char* fileextension,
    FuriString* nextfilename,
    uint8_t max_len);

#ifdef __cplusplus
}
#endif
