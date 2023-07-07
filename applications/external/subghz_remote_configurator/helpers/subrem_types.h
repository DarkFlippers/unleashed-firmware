#pragma once

#include <furi.h>
#include <furi_hal.h>

#define SUBREM_APP_APP_FILE_VERSION 1
#define SUBREM_APP_APP_FILE_TYPE "Flipper SubRem Map file"
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
    SubRemViewIDSubmenu,
    SubRemViewIDWidget,
    SubRemViewIDPopup,
    SubRemViewIDTextInput,
    SubRemViewIDRemote,
    SubRemViewIDEditMenu,
} SubRemViewID;

typedef enum {
    SubRemLoadSubStateNotSet = 0,
    SubRemLoadSubStatePreloaded,
    SubRemLoadSubStateError,
    SubRemLoadSubStateErrorIncorectPath,
    SubRemLoadSubStateErrorNoFile,
    SubRemLoadSubStateErrorFreq,
    SubRemLoadSubStateErrorMod,
    SubRemLoadSubStateErrorProtocol,
    SubRemLoadSubStateOK,
} SubRemLoadSubState;

typedef enum {
    SubRemLoadMapStateBack = 0,
    SubRemLoadMapStateError,
    SubRemLoadMapStateErrorOpenError,
    SubRemLoadMapStateErrorStorage,
    SubRemLoadMapStateErrorBrokenFile,
    SubRemLoadMapStateNotAllOK,
    SubRemLoadMapStateOK,
} SubRemLoadMapState;