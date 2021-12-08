#pragma once

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Bt Bt;

typedef enum {
    BtProfileSerial,
    BtProfileHidKeyboard,
} BtProfile;

/**
 * Change BLE Profile
 * @note Call of this function leads to 2nd core restart
 *
 * @param bt        Bt instance
 * @param profile   BtProfile
 *
 * @return          true on success
 */
bool bt_set_profile(Bt* bt, BtProfile profile);

#ifdef __cplusplus
}
#endif
