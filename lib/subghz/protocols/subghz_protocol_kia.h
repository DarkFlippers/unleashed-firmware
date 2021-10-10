#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolKIA SubGhzProtocolKIA;

/** Allocate SubGhzProtocolKIA
 * 
 * @return SubGhzProtocolKIA* 
 */
SubGhzProtocolKIA* subghz_protocol_kia_alloc();

/** Free SubGhzProtocolKIA
 * 
 * @param instance 
 */
void subghz_protocol_kia_free(SubGhzProtocolKIA* instance);

/** Reset internal state
 * @param instance - SubGhzProtocolKIA instance
 */
void subghz_protocol_kia_reset(SubGhzProtocolKIA* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolKIA instance
 */
void subghz_protocol_kia_check_remote_controller(SubGhzProtocolKIA* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolKIA instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_kia_parse(SubGhzProtocolKIA* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolKIA* instance
 * @param output   - output string
 */
void subghz_protocol_kia_to_str(SubGhzProtocolKIA* instance, string_t output);

/** Get a string to save the protocol
 * 
 * @param instance  - SubGhzProtocolKIA instance
 * @param output    - the resulting string
 */
void subghz_protocol_kia_to_save_str(SubGhzProtocolKIA* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolKIA instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_kia_to_load_protocol(SubGhzProtocolKIA* instance, void* context);