#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FURI_HAL_RANDOM_MAX 0xFFFFFFFFU

/** Initialize random subsystem */
void furi_hal_random_init(void);

/** Get random value
 * furi_hal_random_get() gives up to FURI_HAL_RANDOM_MAX
 * rand() and random() give up to RAND_MAX
 *
 * @return     32 bit random value (up to FURI_HAL_RANDOM_MAX)
 */
uint32_t furi_hal_random_get(void);

/** Fill buffer with random data
 *
 * @param      buf  buffer pointer
 * @param      data buffer len
 */
void furi_hal_random_fill_buf(uint8_t* buf, uint32_t len);

#ifdef __cplusplus
}
#endif
