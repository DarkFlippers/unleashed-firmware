#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* Init VCP HAL
 * Allocates ring buffer and initializes state
 */
void api_hal_vcp_init();

/* Recieve data from VCP
 * Waits till some data arrives, never returns 0
 * @param buffer - pointer to buffer
 * @param size - buffer size
 * @return items copied in buffer, 0 if channel closed
 */
size_t api_hal_vcp_rx(uint8_t* buffer, size_t size);

/* Transmit data to VCP
 * @param buffer - pointer to buffer
 * @param size - buffer size
 */
void api_hal_vcp_tx(uint8_t* buffer, size_t size);
