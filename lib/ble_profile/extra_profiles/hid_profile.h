#pragma once

#include <furi_ble/profile_interface.h>

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Optional arguments to pass along with profile template as 
 * FuriHalBleProfileParams for tuning profile behavior 
 **/
typedef struct {
    const char* device_name_prefix; /**< Prefix for device name. Length must be less than 8 */
    uint16_t mac_xor; /**< XOR mask for device address, for uniqueness */
} BleProfileHidParams;

/** Hid Keyboard Profile descriptor */
extern const FuriHalBleProfileTemplate* ble_profile_hid;

/** Press keyboard button
 *
 * @param profile   profile instance
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool ble_profile_hid_kb_press(FuriHalBleProfileBase* profile, uint16_t button);

/** Release keyboard button
 *
 * @param profile   profile instance
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool ble_profile_hid_kb_release(FuriHalBleProfileBase* profile, uint16_t button);

/** Release all keyboard buttons
 *
 * @param profile   profile instance
 * @return          true on success
 */
bool ble_profile_hid_kb_release_all(FuriHalBleProfileBase* profile);

/** Set the following consumer key to pressed state and send HID report
 *
 * @param profile   profile instance
 * @param button    key code
 */
bool ble_profile_hid_consumer_key_press(FuriHalBleProfileBase* profile, uint16_t button);

/** Set the following consumer key to released state and send HID report
 *
 * @param profile   profile instance
 * @param button    key code
 */
bool ble_profile_hid_consumer_key_release(FuriHalBleProfileBase* profile, uint16_t button);

/** Set consumer key to released state and send HID report
 *
 * @param profile   profile instance
 * @param button    key code
 */
bool ble_profile_hid_consumer_key_release_all(FuriHalBleProfileBase* profile);

/** Set mouse movement and send HID report
 *
 * @param profile    profile instance
 * @param      dx    x coordinate delta
 * @param      dy    y coordinate delta
 */
bool ble_profile_hid_mouse_move(FuriHalBleProfileBase* profile, int8_t dx, int8_t dy);

/** Set mouse button to pressed state and send HID report
 *
 * @param profile   profile instance
 * @param   button  key code
 */
bool ble_profile_hid_mouse_press(FuriHalBleProfileBase* profile, uint8_t button);

/** Set mouse button to released state and send HID report
 *
 * @param profile   profile instance
 * @param   button  key code
 */
bool ble_profile_hid_mouse_release(FuriHalBleProfileBase* profile, uint8_t button);

/** Set mouse button to released state and send HID report
 *
 * @param profile   profile instance
 * @param   button  key code
 */
bool ble_profile_hid_mouse_release_all(FuriHalBleProfileBase* profile);

/** Set mouse wheel position and send HID report
 *
 * @param profile   profile instance
 * @param    delta  number of scroll steps
 */
bool ble_profile_hid_mouse_scroll(FuriHalBleProfileBase* profile, int8_t delta);

#ifdef __cplusplus
}
#endif
