#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolScherKhan SubGhzProtocolScherKhan;

/** Allocate SubGhzProtocolScherKhan
 * 
 * @return SubGhzProtocolScherKhan* 
 */
SubGhzProtocolScherKhan* subghz_protocol_scher_khan_alloc();

/** Free SubGhzProtocolScherKhan
 * 
 * @param instance 
 */
void subghz_protocol_scher_khan_free(SubGhzProtocolScherKhan* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolScherKhan instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_scher_khan_send_key(SubGhzProtocolScherKhan* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolScherKhan instance
 */
void subghz_protocol_scher_khan_reset(SubGhzProtocolScherKhan* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolScherKhan instance
 */
void subghz_protocol_scher_khan_check_remote_controller(SubGhzProtocolScherKhan* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolScherKhan instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_scher_khan_parse(SubGhzProtocolScherKhan* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolScherKhan* instance
 * @param output   - output string
 */
void subghz_protocol_scher_khan_to_str(SubGhzProtocolScherKhan* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolScherKhan instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_scher_khan_to_load_protocol(SubGhzProtocolScherKhan* instance, void* context);
