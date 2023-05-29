#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize random subsystem */
void furi_hal_random_init();

/** Get random value
 *
 * @return     random value
 */
uint32_t furi_hal_random_get();

/** Fill buffer with random data
 *
 * @param      buf  buffer pointer
 * @param      data buffer len
 */
void furi_hal_random_fill_buf(uint8_t* buf, uint32_t len);

#ifdef __cplusplus
}
#endif
