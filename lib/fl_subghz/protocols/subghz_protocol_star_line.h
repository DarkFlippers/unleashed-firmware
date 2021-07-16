#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzKeystore SubGhzKeystore;

typedef struct SubGhzProtocolStarLine SubGhzProtocolStarLine;

/** Allocate SubGhzProtocolStarLine
 * 
 * @return SubGhzProtocolStarLine* 
 */
SubGhzProtocolStarLine* subghz_protocol_star_line_alloc(SubGhzKeystore* keystore);

/** Free SubGhzProtocolStarLine
 * 
 * @param instance 
 */
void subghz_protocol_star_line_free(SubGhzProtocolStarLine* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolStarLine instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_star_line_send_key(SubGhzProtocolStarLine* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolStarLine instance
 */
void subghz_protocol_star_line_reset(SubGhzProtocolStarLine* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolStarLine instance
 */
void subghz_protocol_star_line_check_remote_controller(SubGhzProtocolStarLine* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolStarLine instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_star_line_parse(SubGhzProtocolStarLine* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolStarLine* instance
 * @param output   - output string
 */
void subghz_protocol_star_line_to_str(SubGhzProtocolStarLine* instance, string_t output);
