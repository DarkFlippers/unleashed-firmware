#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <lib/flipper_format/flipper_format.h>
#include <furi.h>
#include <furi_hal.h>
#include "../types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SubGhzBlockGeneric SubGhzBlockGeneric;

struct SubGhzBlockGeneric {
    const char* protocol_name;
    uint64_t data;
    uint64_t data_2;
    uint32_t serial;
    uint16_t data_count_bit;
    uint8_t btn;
    uint32_t cnt;
    uint8_t cnt_2;
    uint32_t seed;
};

typedef struct SubGhzBlockGenericGlobal SubGhzBlockGenericGlobal;

struct SubGhzBlockGenericGlobal {
    uint32_t current_cnt; // current counter value;
    uint32_t new_cnt; // new counter value;
    bool cnt_need_override; // flag for protocols to override signals counter inside of protocols
    uint8_t cnt_length_bit; // counter length in bits (used in counter editor giu)
    bool cnt_is_available; // is there counter available for protocol (used in counter editor giu)

    uint8_t current_btn; // current button value;
    uint8_t new_btn; // new button value;
    bool btn_need_override; // flag for protocols to override button inside of protocols
    uint8_t btn_length_bit; // button length in bits (used in counter editor giu)
    bool btn_is_available; // is there button available for protocol (used in button editor giu)

    bool endless_tx; // used for endless/breakless transmission in subghz protols yield function (when user hold OK button)
};

extern SubGhzBlockGenericGlobal subghz_block_generic_global; //global structure for subghz

/**
 * Setup new_cnt and cnt_need_override flag to be used in protocols;
 * @param counter new counter value;
 */
void subghz_block_generic_global_counter_override_set(uint32_t counter);

/**
 * Return true if incomming variable was overrided by new_cnt
 * else return false and not change incomming variable
 * @param counter pointer to counter variable that must be changed
 */
bool subghz_block_generic_global_counter_override_get(uint32_t* counter);

/**
 * Setup new_btn and btn_need_override flag to be used in protocols;
 * @param button new button value;
 */
void subghz_block_generic_global_button_override_set(uint8_t button);

/**
 * Return true if incomming variable was overrided by new_btn
 * else return false and not change incomming variable
 * @param button pointer to counter variable that must be changed
 */
bool subghz_block_generic_global_button_override_get(uint8_t* button);

/**
 * Reset subghz_block_generic global structure;
 */
void subghz_block_generic_global_reset(void* p);

/**
 * Get name preset.
 * @param preset_name name preset
 * @param preset_str Output name preset
 */
void subghz_block_generic_get_preset_name(const char* preset_name, FuriString* preset_str);

/**
 * Serialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return Status Error
 */
SubGhzProtocolStatus subghz_block_generic_serialize(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return Status Error
 */
SubGhzProtocolStatus
    subghz_block_generic_deserialize(SubGhzBlockGeneric* instance, FlipperFormat* flipper_format);

/**
 * Deserialize data SubGhzBlockGeneric.
 * @param instance Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param count_bit Count bit protocol
 * @return Status Error
 */
SubGhzProtocolStatus subghz_block_generic_deserialize_check_count_bit(
    SubGhzBlockGeneric* instance,
    FlipperFormat* flipper_format,
    uint16_t count_bit);

#ifdef __cplusplus
}
#endif
