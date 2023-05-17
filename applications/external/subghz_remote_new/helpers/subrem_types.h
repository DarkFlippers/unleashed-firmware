#pragma once

#include <furi.h>
#include <furi_hal.h>

// #define SUBREM_APP_APP_FILE_VERSION 1
// #define SUBREM_APP_APP_FILE_TYPE "Flipper SubRem Map file"
#define SUBREM_APP_EXTENSION ".txt"

typedef enum {
    SubRemSubKeyNameUp = (0U),
    SubRemSubKeyNameDown,
    SubRemSubKeyNameLeft,
    SubRemSubKeyNameRight,
    SubRemSubKeyNameOk,
    SubRemSubKeyNameMaxCount,
} SubRemSubKeyName;

typedef enum {
    SubRemViewSubmenu,
    SubRemViewWidget,
    SubRemViewPopup,
    SubRemViewTextInput,
    SubRemViewIDRemote,
} SubRemViewID;

typedef enum {
    // Loadin State
    SubRemSubKeyTypeNoData = 0,
    // SubRemSubKeyTypeHaveFileName,

    // Key Type
    SubRemSubKeyTypeStaticKey = 100,
    SubRemSubKeyTypeDynamicKey,
    SubRemSubKeyTypeRawKey,
} SubRemSubKeyType; // TODO: depricated

// typedef enum {
//
// } SubRemLoadMapState;