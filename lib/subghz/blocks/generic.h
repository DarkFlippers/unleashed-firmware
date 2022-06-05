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
    uint64_t data_2;
    uint32_t serial;
    uint8_t data_count_bit;
    uint8_t btn;
    uint32_t cnt;
    uint8_t cnt_2;
    uint32_t seed;
};

/**
 * Get modulation name.
 * @param preset modulation,FuriHalSubGhzPreset 
 * @param preset_str Output modulation name 
 * @return true On success
 */
bool subghz_block_generic_get_preset_name(FuriHalSubGhzPreset preset, string_t preset_str);

/**
 * Serialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param frequency The frequency at which the signal was received, Hz
 * @param preset The modulation on which the signal was received, FuriHalSubGhzPreset
 * @return true On success
 */
bool subghz_block_generic_serialize(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

/**
 * Deserialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_block_generic_deserialize(SubGhzBlockGeneric* instance, FlipperFormat* flipper_format);
