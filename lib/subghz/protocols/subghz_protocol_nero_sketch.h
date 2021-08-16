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

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param encoder - SubGhzProtocolEncoderCommon encoder
 * @return bool
 */
bool subghz_protocol_nero_sketch_send_key(SubGhzProtocolNeroSketch* instance, SubGhzProtocolEncoderCommon* encoder);

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

void subghz_protocol_nero_sketch_to_save_str(SubGhzProtocolNeroSketch* instance, string_t output);
bool subghz_protocol_nero_sketch_to_load_protocol(FileWorker* file_worker, SubGhzProtocolNeroSketch* instance);
