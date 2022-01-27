#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolSomfyKeytis SubGhzProtocolSomfyKeytis;

/** Allocate SubGhzProtocolSomfyKeytis
 * 
 * @return SubGhzProtocolSomfyKeytis* 
 */
SubGhzProtocolSomfyKeytis* subghz_protocol_somfy_keytis_alloc();

/** Free SubGhzProtocolSomfyKeytis
 * 
 * @param instance 
 */
void subghz_protocol_somfy_keytis_free(SubGhzProtocolSomfyKeytis* instance);

uint32_t subghz_protocol_somfy_keytis_get_press_duration(void* context);

/** Reset internal state
 * @param instance - SubGhzProtocolSomfyKeytis instance
 */
void subghz_protocol_somfy_keytis_reset(SubGhzProtocolSomfyKeytis* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolSomfyKeytis instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_somfy_keytis_parse(
    SubGhzProtocolSomfyKeytis* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolSomfyKeytis* instance
 * @param output   - output string
 */
void subghz_protocol_somfy_keytis_to_str(SubGhzProtocolSomfyKeytis* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolSomfyKeytis instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_somfy_keytis_to_load_protocol(
    SubGhzProtocolSomfyKeytis* instance,
    void* context);