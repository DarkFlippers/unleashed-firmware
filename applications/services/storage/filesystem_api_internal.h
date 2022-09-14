#pragma once
#include <furi.h>
#include "filesystem_api_defines.h"

#ifdef __cplusplus
extern "C" {
#endif

/** File type */
typedef enum {
    FileTypeClosed, /**< Closed file */
    FileTypeOpenDir, /**< Open dir */
    FileTypeOpenFile, /**< Open file */
} FileType;

/** Structure that hold file index and returned api errors */
struct File {
    uint32_t file_id; /**< File ID for internal references */
    FileType type;
    FS_Error error_id; /**< Standart API error from FS_Error enum */
    int32_t internal_error_id; /**< Internal API error value */
    void* storage;
};

/** File api structure
 *  @var FS_File_Api::open
 *      @brief Open file
 *      @param file pointer to file object, filled by api
 *      @param path path to file 
 *      @param access_mode access mode from FS_AccessMode 
 *      @param open_mode open mode from FS_OpenMode 
 *      @return success flag
 * 
 *  @var FS_File_Api::close 
 *      @brief Close file
 *      @param file pointer to file object
 *      @return success flag
 * 
 *  @var FS_File_Api::read
 *      @brief Read bytes from file to buffer
 *      @param file pointer to file object
 *      @param buff pointer to buffer for reading
 *      @param bytes_to_read how many bytes to read, must be smaller or equal to buffer size 
 *      @return how many bytes actually has been read
 * 
 *  @var FS_File_Api::write
 *      @brief Write bytes from buffer to file
 *      @param file pointer to file object
 *      @param buff pointer to buffer for writing
 *      @param bytes_to_read how many bytes to write, must be smaller or equal to buffer size 
 *      @return how many bytes actually has been written
 * 
 *  @var FS_File_Api::seek
 *      @brief Move r/w pointer 
 *      @param file pointer to file object
 *      @param offset offset to move r/w pointer
 *      @param from_start set offset from start, or from current position
 *      @return success flag
 * 
 *  @var FS_File_Api::tell
 *      @brief Get r/w pointer position
 *      @param file pointer to file object
 *      @return current r/w pointer position
 * 
 *  @var FS_File_Api::truncate
 *      @brief Truncate file size to current r/w pointer position
 *      @param file pointer to file object
 *      @return success flag
 * 
 *  @var FS_File_Api::size
 *      @brief Fet file size
 *      @param file pointer to file object
 *      @return file size
 * 
 *  @var FS_File_Api::sync
 *      @brief Write file cache to storage
 *      @param file pointer to file object
 *      @return success flag
 * 
 *  @var FS_File_Api::eof
 *      @brief Checks that the r/w pointer is at the end of the file
 *      @param file pointer to file object
 *      @return end of file flag
 */
typedef struct {
    bool (*const open)(
        void* context,
        File* file,
        const char* path,
        FS_AccessMode access_mode,
        FS_OpenMode open_mode);
    bool (*const close)(void* context, File* file);
    uint16_t (*read)(void* context, File* file, void* buff, uint16_t bytes_to_read);
    uint16_t (*write)(void* context, File* file, const void* buff, uint16_t bytes_to_write);
    bool (*const seek)(void* context, File* file, uint32_t offset, bool from_start);
    uint64_t (*tell)(void* context, File* file);
    bool (*const truncate)(void* context, File* file);
    uint64_t (*size)(void* context, File* file);
    bool (*const sync)(void* context, File* file);
    bool (*const eof)(void* context, File* file);
} FS_File_Api;

/** Dir api structure
 *  @var FS_Dir_Api::open
 *      @brief Open directory to get objects from
 *      @param file pointer to file object, filled by api
 *      @param path path to directory 
 *      @return success flag
 * 
 *  @var FS_Dir_Api::close 
 *      @brief Close directory
 *      @param file pointer to file object
 *      @return success flag
 * 
 *  @var FS_Dir_Api::read
 *      @brief Read next object info in directory
 *      @param file pointer to file object
 *      @param fileinfo pointer to read FileInfo, can be NULL
 *      @param name pointer to name buffer, can be NULL
 *      @param name_length name buffer length
 *      @return success flag (if next object not exist also returns false and set error_id to FSE_NOT_EXIST)
 * 
 *  @var FS_Dir_Api::rewind
 *      @brief Rewind to first object info in directory
 *      @param file pointer to file object
 *      @return success flag
 */
typedef struct {
    bool (*const open)(void* context, File* file, const char* path);
    bool (*const close)(void* context, File* file);
    bool (*const read)(
        void* context,
        File* file,
        FileInfo* fileinfo,
        char* name,
        uint16_t name_length);
    bool (*const rewind)(void* context, File* file);
} FS_Dir_Api;

/** Common api structure
 *  @var FS_Common_Api::stat
 *      @brief Open directory to get objects from
 *      @param path path to file/directory
 *      @param fileinfo pointer to read FileInfo, can be NULL
 *      @param name pointer to name buffer, can be NULL
 *      @param name_length name buffer length
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::remove
 *      @brief Remove file/directory from storage, 
 *          directory must be empty,
 *          file/directory must not be opened,
 *          file/directory must not have FSF_READ_ONLY flag
 *      @param path path to file/directory
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::mkdir
 *      @brief Create new directory
 *      @param path path to new directory
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::fs_info
 *      @brief Get total and free space storage values
 *      @param fs_path path of fs
 *      @param total_space pointer to total space value
 *      @param free_space pointer to free space value
 *      @return FS_Error error info
 */
typedef struct {
    FS_Error (*const stat)(void* context, const char* path, FileInfo* fileinfo);
    FS_Error (*const remove)(void* context, const char* path);
    FS_Error (*const mkdir)(void* context, const char* path);
    FS_Error (*const fs_info)(
        void* context,
        const char* fs_path,
        uint64_t* total_space,
        uint64_t* free_space);
} FS_Common_Api;

/** Full filesystem api structure */
typedef struct {
    const FS_File_Api file;
    const FS_Dir_Api dir;
    const FS_Common_Api common;
} FS_Api;

#ifdef __cplusplus
}
#endif
