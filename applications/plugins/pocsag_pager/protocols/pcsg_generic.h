#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/flipper_format/flipper_format.h>
#include "furi.h"
#include "furi_hal.h"
#include <lib/subghz/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PCSGBlockGeneric PCSGBlockGeneric;

struct PCSGBlockGeneric {
    const char* protocol_name;
    FuriString* result_ric;
    FuriString* result_msg;
};

/**
 * Get name preset.
 * @param preset_name name preset
 * @param preset_str Output name preset
 */
void pcsg_block_generic_get_preset_name(const char* preset_name, FuriString* preset_str);

/**
 * Serialize data PCSGBlockGeneric.
 * @param instance Pointer to a PCSGBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return true On success
 */
SubGhzProtocolStatus pcsg_block_generic_serialize(
    PCSGBlockGeneric* instance,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data PCSGBlockGeneric.
 * @param instance Pointer to a PCSGBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    pcsg_block_generic_deserialize(PCSGBlockGeneric* instance, FlipperFormat* flipper_format);

float pcsg_block_generic_fahrenheit_to_celsius(float fahrenheit);

#ifdef __cplusplus
}
#endif