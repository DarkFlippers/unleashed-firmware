#pragma once

#include "subrem_types.h"
#include <flipper_format/flipper_format_i.h>
#include <lib/subghz/protocols/protocol_items.h>

typedef struct {
    uint32_t frequency;
    uint8_t* data;
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

SubRemSubFilePreset* subrem_sub_file_preset_alloc();

void subrem_sub_file_preset_free(SubRemSubFilePreset* sub_preset);

void subrem_sub_file_preset_reset(SubRemSubFilePreset* sub_preset);
