#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/flipper_format/flipper_format.h>
#include "furi.h"
#include "furi_hal.h"
#include "../types.h"

typedef struct SubGhzBlockGeneric SubGhzBlockGeneric;

struct SubGhzBlockGeneric {
    const char* protocol_name;
    uint64_t data;
    uint32_t serial;
    uint8_t data_count_bit;
    uint8_t btn;
    uint32_t cnt;
};

/**
 * Get name preset.
 * @param preset_name name preset
 * @param preset_str Output name preset
 */
void subghz_block_generic_get_preset_name(const char* preset_name, string_t preset_str);

/**
 * Serialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_block_generic_serialize(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_block_generic_deserialize(SubGhzBlockGeneric* instance, FlipperFormat* flipper_format);
