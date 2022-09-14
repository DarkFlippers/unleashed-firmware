#pragma once

#include "serial_service.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FURI_HAL_BT_SERIAL_PACKET_SIZE_MAX SERIAL_SVC_DATA_LEN_MAX

/** Serial service callback type */
typedef SerialServiceEventCallback FuriHalBtSerialCallback;

/** Start Serial Profile
 */
void furi_hal_bt_serial_start();

/** Stop Serial Profile
 */
void furi_hal_bt_serial_stop();

/** Set Serial service events callback
 *
 * @param buffer_size   Applicaition buffer size
 * @param calback       FuriHalBtSerialCallback instance
 * @param context       pointer to context
 */
void furi_hal_bt_serial_set_event_callback(
    uint16_t buff_size,
    FuriHalBtSerialCallback callback,
    void* context);

/** Notify that application buffer is empty
 */
void furi_hal_bt_serial_notify_buffer_is_empty();

/** Send data through BLE
 *
 * @param data  data buffer
 * @param size  data buffer size
 *
 * @return      true on success
 */
bool furi_hal_bt_serial_tx(uint8_t* data, uint16_t size);

#ifdef __cplusplus
}
#endif
