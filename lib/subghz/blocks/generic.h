#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/flipper_format/flipper_format.h>
#include "furi.h"
#include "furi_hal.h"

typedef struct SubGhzBlockGeneric SubGhzBlockGeneric;

struct SubGhzBlockGeneric {
    const char* protocol_name;
    uint64_t data;
    uint32_t serial;
    uint8_t data_count_bit;
    uint8_t btn;
    uint16_t cnt;
};

bool subghz_block_generic_get_preset_name(FuriHalSubGhzPreset preset, string_t preset_str);

bool subghz_block_generic_serialize(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

bool subghz_block_generic_deserialize(SubGhzBlockGeneric* instance, FlipperFormat* flipper_format);
