#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolPrinceton SubGhzProtocolPrinceton;

/** Allocate SubGhzProtocolPrinceton
 * 
 * @return SubGhzProtocolPrinceton* 
 */
SubGhzProtocolPrinceton* subghz_protocol_princeton_alloc();

/** Free SubGhzProtocolPrinceton
 * 
 * @param instance 
 */
void subghz_protocol_princeton_free(SubGhzProtocolPrinceton* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolPrinceton instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_princeton_send_key(SubGhzProtocolPrinceton* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolPrinceton instance
 */
void subghz_protocol_princeton_reset(SubGhzProtocolPrinceton* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolPrinceton instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_princeton_parse(SubGhzProtocolPrinceton* instance, bool level, uint32_t duration);

