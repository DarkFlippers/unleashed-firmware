#pragma once
#include <furi.h>

#define SORTING_MAX_NAME_LENGTH 255
#define SORTING_MIN_FREE_MEMORY (1024 * 40)

/**
 * @brief Sorted file record, holds file name and info
 */
typedef struct {
    FuriString* name;
    FileInfo info;
} SortedFileRecord;

/**
 * @brief Sorted directory, holds sorted file records, count and current index
 */
typedef struct {
    SortedFileRecord* sorted;
    size_t count;
    size_t index;
} SortedDir;