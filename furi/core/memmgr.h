/**
 * @file memmgr.h
 * Furi: memory management API and glue
 */

#pragma once

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "check.h"

#ifdef __cplusplus
extern "C" {
#endif

// define for test case "link against furi memmgr"
#define FURI_MEMMGR_GUARD 1

/** Get free heap size
 *
 * @return     free heap size in bytes
 */
size_t memmgr_get_free_heap(void);

/** Get total heap size
 *
 * @return     total heap size in bytes
 */
size_t memmgr_get_total_heap(void);

/** Get heap watermark
 *
 * @return     minimum heap in bytes
 */
size_t memmgr_get_minimum_free_heap(void);

/**
 * @brief Allocate memory from the auxiliary memory pool. That memory can't be freed.
 * 
 * @param size 
 * @return void* 
 */
void* memmgr_aux_pool_alloc(size_t size);

/**
 * @brief Get the auxiliary pool free memory size
 * 
 * @return size_t 
 */
size_t memmgr_aux_pool_get_free(void);

/**
 * @brief Get max free block size from the auxiliary memory pool
 * 
 * @return size_t 
 */
size_t memmgr_pool_get_max_block(void);

#ifdef __cplusplus
}
#endif
