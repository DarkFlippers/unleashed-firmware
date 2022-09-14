#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Access mode flags */
typedef enum {
    FSAM_READ = (1 << 0), /**< Read access */
    FSAM_WRITE = (1 << 1), /**< Write access */
    FSAM_READ_WRITE = FSAM_READ | FSAM_WRITE, /**< Read and write access */
} FS_AccessMode;

/** Open mode flags */
typedef enum {
    FSOM_OPEN_EXISTING = 1, /**< Open file, fail if file doesn't exist */
    FSOM_OPEN_ALWAYS = 2, /**< Open file. Create new file if not exist */
    FSOM_OPEN_APPEND = 4, /**< Open file. Create new file if not exist. Set R/W pointer to EOF */
    FSOM_CREATE_NEW = 8, /**< Creates a new file. Fails if the file is exist */
    FSOM_CREATE_ALWAYS = 16, /**< Creates a new file. If file exist, truncate to zero size */
} FS_OpenMode;

/** API errors enumeration */
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
    FSE_ALREADY_OPEN, /**< File/Dir already opened */
} FS_Error;

/** FileInfo flags */
typedef enum {
    FSF_DIRECTORY = (1 << 0), /**< Directory */
} FS_Flags;

/**  Structure that hold file index and returned api errors  */
typedef struct File File;

/**  Structure that hold file info */
typedef struct {
    uint8_t flags; /**< flags from FS_Flags enum */
    uint64_t size; /**< file size */
} FileInfo;

/** Gets the error text from FS_Error
 * @param error_id error id
 * @return const char* error text
 */
const char* filesystem_api_error_get_desc(FS_Error error_id);

#ifdef __cplusplus
}
#endif
