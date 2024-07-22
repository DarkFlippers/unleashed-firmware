#pragma once
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DirWalk DirWalk;

typedef enum {
    DirWalkOK, /**< OK */
    DirWalkError, /**< Error */
    DirWalkLast, /**< Last element */
} DirWalkResult;

typedef bool (*DirWalkFilterCb)(const char* name, FileInfo* fileinfo, void* ctx);

/**
 * Allocate DirWalk
 * @param storage 
 * @return DirWalk* 
 */
DirWalk* dir_walk_alloc(Storage* storage);

/**
 * Free DirWalk
 * @param dir_walk 
 */
void dir_walk_free(DirWalk* dir_walk);

/**
 * Set recursive mode (true by default)
 * @param dir_walk 
 * @param recursive 
 */
void dir_walk_set_recursive(DirWalk* dir_walk, bool recursive);

/**
 * Set filter callback (Should return true if the data is valid)
 * @param dir_walk 
 * @param cb 
 * @param context 
 */
void dir_walk_set_filter_cb(DirWalk* dir_walk, DirWalkFilterCb cb, void* context);

/**
 * Open directory 
 * @param dir_walk 
 * @param path 
 * @return true 
 * @return false 
 */
bool dir_walk_open(DirWalk* dir_walk, const char* path);

/**
 * Get error id
 * @param dir_walk 
 * @return FS_Error 
 */
FS_Error dir_walk_get_error(DirWalk* dir_walk);

/**
 * Read next element from directory
 * @param dir_walk 
 * @param return_path 
 * @param fileinfo 
 * @return DirWalkResult 
 */
DirWalkResult dir_walk_read(DirWalk* dir_walk, FuriString* return_path, FileInfo* fileinfo);

/**
 * Close directory
 * @param dir_walk 
 */
void dir_walk_close(DirWalk* dir_walk);

#ifdef __cplusplus
}
#endif
