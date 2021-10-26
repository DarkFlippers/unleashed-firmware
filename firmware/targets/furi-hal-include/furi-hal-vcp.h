/**
 * @file furi-hal-vcp.h
 * VCP HAL API
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Init VCP HAL Allocates ring buffer and initializes state
 */
void furi_hal_vcp_init();

/** Disable VCP to make CDC interface usable by other application
 */
void furi_hal_vcp_disable();

/** Enable VCP
 */
void furi_hal_vcp_enable();

/** Recieve data from VCP Waits till some data arrives, never returns 0
 *
 * @param      buffer  pointer to buffer
 * @param      size    buffer size
 *
 * @return     items copied in buffer, 0 if channel closed
 */
size_t furi_hal_vcp_rx(uint8_t* buffer, size_t size);

/** Recieve data from VCP with timeout Waits till some data arrives during
 * timeout
 *
 * @param      buffer   pointer to buffer
 * @param      size     buffer size
 * @param      timeout  rx timeout in ms
 *
 * @return     items copied in buffer, 0 if channel closed or timeout occurs
 */
size_t furi_hal_vcp_rx_with_timeout(uint8_t* buffer, size_t size, uint32_t timeout);

/** Transmit data to VCP
 *
 * @param      buffer  pointer to buffer
 * @param      size    buffer size
 */
void furi_hal_vcp_tx(const uint8_t* buffer, size_t size);

/** Check whether VCP is connected
 *
 * @return     true if connected
 */
bool furi_hal_vcp_is_connected(void);

#ifdef __cplusplus
}
#endif
