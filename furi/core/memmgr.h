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

#ifdef __cplusplus
}
#endif
