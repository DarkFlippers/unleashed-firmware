#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolFaacSLH SubGhzProtocolFaacSLH;

/** Allocate SubGhzProtocolFaacSLH
 * 
 * @return SubGhzProtocolFaacSLH* 
 */
SubGhzProtocolFaacSLH* subghz_protocol_faac_slh_alloc();

/** Free SubGhzProtocolFaacSLH
 * 
 * @param instance 
 */
void subghz_protocol_faac_slh_free(SubGhzProtocolFaacSLH* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolFaacSLH instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_faac_slh_send_key(
    SubGhzProtocolFaacSLH* instance,
    uint64_t key,
    uint8_t bit,
    uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolFaacSLH instance
 */
void subghz_protocol_faac_slh_reset(SubGhzProtocolFaacSLH* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolFaacSLH instance
 */
void subghz_protocol_faac_slh_check_remote_controller(SubGhzProtocolFaacSLH* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolFaacSLH instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_faac_slh_parse(SubGhzProtocolFaacSLH* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolFaacSLH* instance
 * @param output   - output string
 */
void subghz_protocol_faac_slh_to_str(SubGhzProtocolFaacSLH* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolFaacSLH instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_faac_slh_to_load_protocol(SubGhzProtocolFaacSLH* instance, void* context);
