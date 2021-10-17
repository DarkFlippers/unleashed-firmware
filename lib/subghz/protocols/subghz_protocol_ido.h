#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolIDo SubGhzProtocolIDo;

/** Allocate SubGhzProtocolIDo
 * 
 * @return SubGhzProtocolIDo* 
 */
SubGhzProtocolIDo* subghz_protocol_ido_alloc();

/** Free SubGhzProtocolIDo
 * 
 * @param instance 
 */
void subghz_protocol_ido_free(SubGhzProtocolIDo* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolIDo instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_ido_send_key(SubGhzProtocolIDo* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolIDo instance
 */
void subghz_protocol_ido_reset(SubGhzProtocolIDo* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolIDo instance
 */
void subghz_protocol_ido_check_remote_controller(SubGhzProtocolIDo* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolIDo instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_ido_parse(SubGhzProtocolIDo* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolIDo* instance
 * @param output   - output string
 */
void subghz_protocol_ido_to_str(SubGhzProtocolIDo* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolIDo instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_ido_to_load_protocol(SubGhzProtocolIDo* instance, void* context);
