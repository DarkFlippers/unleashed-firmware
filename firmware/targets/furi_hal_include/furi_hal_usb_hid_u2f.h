#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define HID_U2F_PACKET_LEN 64

typedef enum {
    HidU2fDisconnected,
    HidU2fConnected,
    HidU2fRequest,
} HidU2fEvent;

typedef void (*HidU2fCallback)(HidU2fEvent ev, void* context);

/** Get HID U2F connection state
 *
 * @return      true / false
 */
bool furi_hal_hid_u2f_is_connected();

/** Set HID U2F event callback
 *
 * @param      cb  callback
 * @param      ctx  callback context
 */
void furi_hal_hid_u2f_set_callback(HidU2fCallback cb, void* ctx);

/** Get received U2F HID packet
 *
 */
uint32_t furi_hal_hid_u2f_get_request(uint8_t* data);

/** Send U2F HID response packet
 *
 * @param      data  response data
 * @param      len  packet length
 */
void furi_hal_hid_u2f_send_response(uint8_t* data, uint8_t len);

#ifdef __cplusplus
}
#endif
