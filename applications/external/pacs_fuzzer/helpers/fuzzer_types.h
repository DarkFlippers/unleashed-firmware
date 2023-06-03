#pragma once

#include <furi.h>
#include <furi_hal.h>

typedef struct {
    uint8_t menu_index;
    uint8_t proto_index;
} FuzzerState;

typedef enum {
    FuzzerViewIDMain,
    FuzzerViewIDAttack,
    FuzzerViewIDFieldEditor,
} FuzzerViewID;