#pragma once

#include <stdint.h>
#include <stdbool.h>

enum FuriHalBtHidMediKeys {
    FuriHalBtHidMediaScanNext,
    FuriHalBtHidMediaScanPrevious,
    FuriHalBtHidMediaStop,
    FuriHalBtHidMediaEject,
    FuriHalBtHidMediaPlayPause,
    FuriHalBtHidMediaMute,
    FuriHalBtHidMediaVolumeUp,
    FuriHalBtHidMediaVolumeDown,
};

/** Start Hid Keyboard Profile
 */
void furi_hal_bt_hid_start();

/** Stop Hid Keyboard Profile
 */
void furi_hal_bt_hid_stop();

/** Press keyboard button
 *
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_press(uint16_t button);

/** Release keyboard button
 *
 * @param button    button code from HID specification
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_release(uint16_t button);

/** Release all keyboard buttons
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_kb_release_all();

/** Release all media buttons
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_media_press(uint8_t button);

/** Release all media buttons
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_media_release(uint8_t button);

/** Release all media buttons
 *
 * @return          true on success
 */
bool furi_hal_bt_hid_media_release_all();
