#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Bt Bt;

typedef enum {
    BtStatusUnavailable,
    BtStatusOff,
    BtStatusAdvertising,
    BtStatusConnected,
} BtStatus;

typedef enum {
    BtProfileSerial,
    BtProfileHidKeyboard,
} BtProfile;

typedef void (*BtStatusChangedCallback)(BtStatus status, void* context);

/** Change BLE Profile
 * @note Call of this function leads to 2nd core restart
 *
 * @param bt        Bt instance
 * @param profile   BtProfile
 *
 * @return          true on success
 */
bool bt_set_profile(Bt* bt, BtProfile profile);

/** Set callback for Bluetooth status change notification
 *
 * @param bt        Bt instance
 * @param callback  BtStatusChangedCallback instance
 * @param context   pointer to context
 */
void bt_set_status_changed_callback(Bt* bt, BtStatusChangedCallback callback, void* context);

#ifdef __cplusplus
}
#endif
