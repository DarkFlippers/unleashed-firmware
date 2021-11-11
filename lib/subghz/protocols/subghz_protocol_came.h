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
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_came_send_key(
    SubGhzProtocolCame* instance,
    SubGhzProtocolCommonEncoder* encoder);

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

/** Adding data to a file
 * 
 * @param instance  - SubGhzProtocolCame instance
 * @param flipper_file - FlipperFile 
 * @return bool
 */
bool subghz_protocol_came_to_save_file(SubGhzProtocolCame* instance, FlipperFile* flipper_file);

/** Loading protocol from file
 * 
 * @param flipper_file - FlipperFile 
 * @param instance - SubGhzProtocolCame instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_came_to_load_protocol_from_file(
    FlipperFile* flipper_file,
    SubGhzProtocolCame* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_came_to_load_protocol(SubGhzProtocolCame* instance, void* context);
