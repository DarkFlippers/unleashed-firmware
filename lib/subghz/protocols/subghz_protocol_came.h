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

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param encoder - SubGhzProtocolEncoderCommon encoder
 * @return bool
 */
bool subghz_protocol_came_send_key(SubGhzProtocolCame* instance, SubGhzProtocolEncoderCommon* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolCame instance
 */
void subghz_protocol_came_reset(SubGhzProtocolCame* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_came_parse(SubGhzProtocolCame* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolCame* instance
 * @param output   - output string
 */
void subghz_protocol_came_to_str(SubGhzProtocolCame* instance, string_t output);

void subghz_protocol_came_to_save_str(SubGhzProtocolCame* instance, string_t output);
bool subghz_protocol_came_to_load_protocol(FileWorker* file_worker, SubGhzProtocolCame* instance);
