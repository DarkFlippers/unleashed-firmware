#pragma once

#include <ble_profile/extra_profiles/hid_profile.h>

/** 
 * Optional arguments to pass along with profile template as 
 * FuriHalBleProfileParams for tuning profile behavior 
 **/
typedef struct {
    char name[FURI_HAL_BT_ADV_NAME_LENGTH]; /**< Full device name */
} BleProfileHidExtParams;

/** Hid Keyboard Profile descriptor */
extern const FuriHalBleProfileTemplate* ble_profile_hid_ext;
