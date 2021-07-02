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

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolPrinceton instance
 * @param data - LevelPair data
 */
void subghz_protocol_princeton_parse(SubGhzProtocolPrinceton* instance, LevelPair data);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolPrinceton* instance
 * @param output   - output string
 */
//void subghz_protocol_princeton_to_str(SubGhzProtocolPrinceton* instance, string_t output);
