#pragma once

#include "subrem_types.h"
#include "txrx/subghz_txrx.h"

#include <flipper_format/flipper_format_i.h>
#include <lib/subghz/types.h>

typedef struct {
    FuriString* name;
    uint32_t frequency;
    // size_t preset_index; // Need for custom preset
} FreqPreset;

// Sub File preset
typedef struct {
    FlipperFormat* fff_data;
    FreqPreset freq_preset;
    FuriString* file_path;
    FuriString* protocaol_name;
    FuriString* label;
    SubGhzProtocolType type;
    SubRemLoadSubState load_state;
} SubRemSubFilePreset;

typedef struct {
    SubRemSubFilePreset* subs_preset[SubRemSubKeyNameMaxCount];
} SubRemMapPreset;

SubRemSubFilePreset* subrem_sub_file_preset_alloc();

void subrem_sub_file_preset_free(SubRemSubFilePreset* sub_preset);

void subrem_sub_file_preset_reset(SubRemSubFilePreset* sub_preset);

SubRemLoadSubState subrem_sub_preset_load(
    SubRemSubFilePreset* sub_preset,
    SubGhzTxRx* txrx,
    FlipperFormat* fff_data_file);
