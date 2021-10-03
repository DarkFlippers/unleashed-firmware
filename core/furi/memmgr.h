/**
 * @file memmgr.h
 * Furi: memory managment API and glue
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

/** Get heap watermark
 *
 * @return     minimum heap in bytes
 */
size_t memmgr_get_minimum_free_heap(void);

/** Allocate memory from heap
 *
 * @note       performs memset with 0, will crash system if not enough memory
 *
 * @param[in]  size  bytes to allocate
 *
 * @return     pointer to allocated memory
 */
void* furi_alloc(size_t size);

#ifdef __cplusplus
}
#endif
