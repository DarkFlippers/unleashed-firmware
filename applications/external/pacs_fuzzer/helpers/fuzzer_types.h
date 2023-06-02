#pragma once

#include <furi.h>
#include <furi_hal.h>

// TODO replace it
typedef enum {
    FuzzerMainMenuIndexDefaultValues = 0,
    FuzzerMainMenuIndexLoadFile,
    FuzzerMainMenuIndexLoadFileCustomUids,

    FuzzerMainMenuIndexMax,
} FuzzerMainMenuIndex;

typedef enum {
    FuzzerViewIDMain,
} FuzzerViewID;