#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNeroSketch SubGhzProtocolNeroSketch;

/** Allocate SubGhzProtocolNeroSketch
 * 
 * @return SubGhzProtocolNeroSketch* 
 */
SubGhzProtocolNeroSketch* subghz_protocol_nero_sketch_alloc();

/** Free SubGhzProtocolNeroSketch
 * 
 * @param instance 
 */
void subghz_protocol_nero_sketch_free(SubGhzProtocolNeroSketch* instance);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolNeroSketch instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_faac_nero_sketch_key(SubGhzProtocolNeroSketch* instance, uint64_t key, uint8_t bit, uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolNeroSketch instance
 */
void subghz_protocol_nero_sketch_reset(SubGhzProtocolNeroSketch* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolNeroSketch instance
 */
void subghz_protocol_nero_sketch_check_remote_controller(SubGhzProtocolNeroSketch* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolNeroSketch instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_nero_sketch_parse(SubGhzProtocolNeroSketch* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolNeroSketch* instance
 * @param output   - output string
 */
void subghz_protocol_nero_sketch_to_str(SubGhzProtocolNeroSketch* instance, string_t output);
