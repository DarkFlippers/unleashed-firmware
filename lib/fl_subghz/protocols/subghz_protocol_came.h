#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolCame SubGhzProtocolCame;

/** Allocate SubGhzProtocolCame
 * 
 * @return SubGhzProtocolCame* 
 */
SubGhzProtocolCame* subghz_protocol_came_alloc();

/** Free SubGhzProtocolCame
 * 
 * @param instance 
 */
void subghz_protocol_came_free(SubGhzProtocolCame* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_came_send_key(SubGhzProtocolCame* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolCame instance
 */
void subghz_protocol_came_reset(SubGhzProtocolCame* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_came_parse(SubGhzProtocolCame* instance, bool level, uint32_t duration);;
