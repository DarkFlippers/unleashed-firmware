#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolSomfyTelis SubGhzProtocolSomfyTelis;

/** Allocate SubGhzProtocolSomfyTelis
 * 
 * @return SubGhzProtocolSomfyTelis* 
 */
SubGhzProtocolSomfyTelis* subghz_protocol_somfy_telis_alloc();

/** Free SubGhzProtocolSomfyTelis
 * 
 * @param instance 
 */
void subghz_protocol_somfy_telis_free(SubGhzProtocolSomfyTelis* instance);

/** Reset internal state
 * @param instance - SubGhzProtocolSomfyTelis instance
 */
void subghz_protocol_somfy_telis_reset(SubGhzProtocolSomfyTelis* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolSomfyTelis instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_somfy_telis_parse(
    SubGhzProtocolSomfyTelis* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolSomfyTelis* instance
 * @param output   - output string
 */
void subghz_protocol_somfy_telis_to_str(SubGhzProtocolSomfyTelis* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolSomfyTelis instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_somfy_telis_to_load_protocol(SubGhzProtocolSomfyTelis* instance, void* context);