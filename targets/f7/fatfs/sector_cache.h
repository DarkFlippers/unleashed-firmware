#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Init sector cache system
 */
void sector_cache_init(void);

/**
 * @brief Get sector data from cache
 * @param n_sector Sector number
 * @return Pointer to sector data or NULL if not found
 */
uint8_t* sector_cache_get(uint32_t n_sector);

/**
 * @brief Put sector data to cache
 * @param n_sector Sector number
 * @param data Pointer to sector data
 */
void sector_cache_put(uint32_t n_sector, uint8_t* data);

/**
 * @brief Invalidate sector cache for given range
 * @param start_sector Start sector number
 * @param end_sector End sector number
 */
void sector_cache_invalidate_range(uint32_t start_sector, uint32_t end_sector);

#ifdef __cplusplus
}
#endif
