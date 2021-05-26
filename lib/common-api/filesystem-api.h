#pragma once
#include <furi.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 *  @brief Access mode flags
 */
typedef enum {
    FSAM_READ = (1 << 0), /**< Read access */
    FSAM_WRITE = (1 << 1), /**< Write access */
} FS_AccessMode;

/**
 *  @brief Open mode flags
 */
typedef enum {
    FSOM_OPEN_EXISTING = 1, /**< Open file, fail if file doesn't exist */
    FSOM_OPEN_ALWAYS = 2, /**< Open file. Create new file if not exist */
    FSOM_OPEN_APPEND = 4, /**< Open file. Create new file if not exist. Set R/W pointer to EOF */
    FSOM_CREATE_NEW = 8, /**< Creates a new file. Fails if the file is exist */
    FSOM_CREATE_ALWAYS = 16, /**< Creates a new file. If file exist, truncate to zero size */
} FS_OpenMode;

/**
 *  @brief API errors enumeration
 */
typedef enum {
    FSE_OK, /**< No error */
    FSE_NOT_READY, /**< FS not ready */
    FSE_EXIST, /**< File/Dir alrady exist */
    FSE_NOT_EXIST, /**< File/Dir does not exist */
    FSE_INVALID_PARAMETER, /**< Invalid API parameter */
    FSE_DENIED, /**< Access denied */
    FSE_INVALID_NAME, /**< Invalid name/path */
    FSE_INTERNAL, /**< Internal error */
    FSE_NOT_IMPLEMENTED, /**< Functon not implemented */
} FS_Error;

/**
 *  @brief FileInfo flags
 */
typedef enum {
    FSF_READ_ONLY = (1 << 0), /**< Readonly */
    FSF_HIDDEN = (1 << 1), /**< Hidden */
    FSF_SYSTEM = (1 << 2), /**< System */
    FSF_DIRECTORY = (1 << 3), /**< Directory */
    FSF_ARCHIVE = (1 << 4), /**< Archive */
} FS_Flags;

/** 
 *  @brief Structure that hold file index and returned api errors 
 */
typedef struct {
    uint32_t file_id; /**< File ID for internal references */
    FS_Error error_id; /**< Standart API error from FS_Error enum */
    uint32_t internal_error_id; /**< Internal API error value */
} File;

// TODO: solve year 2107 problem
/** 
 *  @brief Structure that hold packed date values 
 */
typedef struct __attribute__((packed)) {
    uint16_t month_day : 5; /**< month day */
    uint16_t month : 4; /**< month index */
    uint16_t year : 7; /**< year, year + 1980 to get actual value */
} FileDate;

/** 
 *  @brief Structure that hold packed time values 
 */
typedef struct __attribute__((packed)) {
    uint16_t second : 5; /**< second, second * 2 to get actual value  */
    uint16_t minute : 6; /**< minute */
    uint16_t hour : 5; /**< hour */
} FileTime;

/** 
 *  @brief Union of simple date and real value 
 */
typedef union {
    FileDate simple; /**< simple access to date */
    uint16_t value; /**< real date value */
} FileDateUnion;

/** 
 *  @brief Union of simple time and real value 
 */
typedef union {
    FileTime simple; /**< simple access to time */
    uint16_t value; /**< real time value */
} FileTimeUnion;

/** 
 *  @brief Structure that hold file info
 */
typedef struct {
    uint8_t flags; /**< flags from FS_Flags enum */
    uint64_t size; /**< file size */
    FileDateUnion date; /**< file date */
    FileTimeUnion time; /**< file time */
} FileInfo;

/** @struct FS_File_Api
 *  @brief File api structure
 * 
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
 *      @return how many bytes actually has been readed
 * 
 *  @var FS_File_Api::write
 *      @brief Write bytes from buffer to file
 *      @param file pointer to file object
 *      @param buff pointer to buffer for writing
 *      @param bytes_to_read how many bytes to write, must be smaller or equal to buffer size 
 *      @return how many bytes actually has been writed
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

/**
 * @brief File api structure
 */
typedef struct {
    bool (*open)(File* file, const char* path, FS_AccessMode access_mode, FS_OpenMode open_mode);
    bool (*close)(File* file);
    uint16_t (*read)(File* file, void* buff, uint16_t bytes_to_read);
    uint16_t (*write)(File* file, const void* buff, uint16_t bytes_to_write);
    bool (*seek)(File* file, uint32_t offset, bool from_start);
    uint64_t (*tell)(File* file);
    bool (*truncate)(File* file);
    uint64_t (*size)(File* file);
    bool (*sync)(File* file);
    bool (*eof)(File* file);
} FS_File_Api;

/** @struct FS_Dir_Api
 *  @brief Dir api structure
 * 
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
 *      @param fileinfo pointer to readed FileInfo, can be NULL
 *      @param name pointer to name buffer, can be NULL
 *      @param name_length name buffer length
 *      @return success flag (if next object not exist also returns false and set error_id to FSE_NOT_EXIST)
 * 
 *  @var FS_Dir_Api::rewind
 *      @brief Rewind to first object info in directory
 *      @param file pointer to file object
 *      @return success flag
 */

/**
 * @brief Dir api structure
 */
typedef struct {
    bool (*open)(File* file, const char* path);
    bool (*close)(File* file);
    bool (*read)(File* file, FileInfo* fileinfo, char* name, uint16_t name_length);
    bool (*rewind)(File* file);
} FS_Dir_Api;

/** @struct FS_Common_Api
 *  @brief Common api structure
 * 
 *  @var FS_Common_Api::info
 *      @brief Open directory to get objects from
 *      @param path path to file/directory
 *      @param fileinfo pointer to readed FileInfo, can be NULL
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
 *  @var FS_Common_Api::rename
 *      @brief Rename file/directory,
 *          file/directory must not be opened
 *      @param path path to file/directory
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::set_attr
 *      @brief Set attributes of file/directory, 
 *          for example:
 *          @code
 *          set "read only" flag and remove "hidden" flag
 *          set_attr("file.txt", FSF_READ_ONLY, FSF_READ_ONLY | FSF_HIDDEN);
 *          @endcode
 *      @param path path to file/directory
 *      @param attr attribute values consist of FS_Flags
 *      @param mask attribute mask consist of FS_Flags
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::mkdir
 *      @brief Create new directory
 *      @param path path to new directory
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::set_time
 *      @brief Set file/directory modification time
 *      @param path path to file/directory
 *      @param date modification date 
 *      @param time modification time
 *      @see FileDateUnion
 *      @see FileTimeUnion
 *      @return FS_Error error info
 * 
 *  @var FS_Common_Api::get_fs_info
 *      @brief Get total and free space storage values
 *      @param total_space pointer to total space value
 *      @param free_space pointer to free space value
 *      @return FS_Error error info
 */

/**
 * @brief Common api structure
 */
typedef struct {
    FS_Error (*info)(const char* path, FileInfo* fileinfo, char* name, const uint16_t name_length);
    FS_Error (*remove)(const char* path);
    FS_Error (*rename)(const char* old_path, const char* new_path);
    FS_Error (*set_attr)(const char* path, uint8_t attr, uint8_t mask);
    FS_Error (*mkdir)(const char* path);
    FS_Error (*set_time)(const char* path, FileDateUnion date, FileTimeUnion time);
    FS_Error (*get_fs_info)(uint64_t* total_space, uint64_t* free_space);
} FS_Common_Api;

/** @struct FS_Error_Api
 *  @brief Errors api structure
 * 
 *  @var FS_Error_Api::get_desc
 *      @brief Get error description text
 *      @param error_id FS_Error error id (for fire/dir functions result can be obtained from File.error_id)
 *      @return pointer to description text
 * 
 *  @var FS_Error_Api::get_internal_desc
 *      @brief Get internal error description text
 *      @param internal_error_id error id (for fire/dir functions result can be obtained from File.internal_error_id)
 *      @return pointer to description text
 */

/**
 * @brief Errors api structure
 */
typedef struct {
    const char* (*get_desc)(FS_Error error_id);
    const char* (*get_internal_desc)(uint32_t internal_error_id);
} FS_Error_Api;

/**
 * @brief Full filesystem api structure
 */
typedef struct {
    FS_File_Api file;
    FS_Dir_Api dir;
    FS_Common_Api common;
    FS_Error_Api error;
} FS_Api;

#ifdef __cplusplus
}
#endif