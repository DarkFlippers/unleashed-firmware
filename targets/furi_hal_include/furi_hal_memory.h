/**
 * @file furi_hal_memory.h
 * Memory HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init memory pool manager
 */
void furi_hal_memory_init(void);

/**
 * @brief Allocate memory from separate memory pool. That memory can't be freed.
 * 
 * @param size 
 * @return void* 
 */
void* furi_hal_memory_alloc(size_t size);

/**
 * @brief Get free memory pool size
 * 
 * @return size_t 
 */
size_t furi_hal_memory_get_free(void);

/**
 * @brief Get max free block size from memory pool
 * 
 * @return size_t 
 */
size_t furi_hal_memory_max_pool_block(void);

#ifdef __cplusplus
}
#endif
