#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Configure for CRC32 calculation 
 * @param synchronize enforce acquisition & release in multithreaded environment
 */
void furi_hal_crc_init(bool synchronize);

/** Blocking call to get control of CRC block. Mandatory while RTOS is running
 * @param timeout time to wait for CRC to be available. Can be osWaitForever
 * @return bool acquisition success 
 */
bool furi_hal_crc_acquire(uint32_t timeout);

/** Reset current calculation state and release CRC block
 */
void furi_hal_crc_reset();

/** Process data block. Does not reset current state,
 * allowing to process arbitrary data lengths
 * @param data pointer to data
 * @param length data length
 * @return uint32_t CRC32 value
 */
uint32_t furi_hal_crc_feed(void* data, uint16_t length);

#ifdef __cplusplus
}
#endif
