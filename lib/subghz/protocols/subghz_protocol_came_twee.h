#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolCameTwee SubGhzProtocolCameTwee;

/** Allocate SubGhzProtocolCameTwee
 * 
 * @return SubGhzProtocolCameTwee* 
 */
SubGhzProtocolCameTwee* subghz_protocol_came_twee_alloc();

/** Free SubGhzProtocolCameTwee
 * 
 * @param instance 
 */
void subghz_protocol_came_twee_free(SubGhzProtocolCameTwee* instance);

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolCameTwee instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_came_twee_send_key(
    SubGhzProtocolCameTwee* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolCameTwee instance
 */
void subghz_protocol_came_twee_reset(SubGhzProtocolCameTwee* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolCameTwee instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_came_twee_parse(
    SubGhzProtocolCameTwee* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolCameTwee* instance
 * @param output   - output string
 */
void subghz_protocol_came_twee_to_str(SubGhzProtocolCameTwee* instance, string_t output);

/** Adding data to a file
 * 
 * @param instance  - SubGhzProtocolCameTwee instance
 * @param flipper_file - FlipperFile 
 * @return bool
 */
bool subghz_protocol_came_twee_to_save_file(
    SubGhzProtocolCameTwee* instance,
    FlipperFile* flipper_file);

/** Loading protocol from file
 * 
 * @param flipper_file - FlipperFile 
 * @param instance - SubGhzProtocolCameTwee instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_came_twee_to_load_protocol_from_file(
    FlipperFile* flipper_file,
    SubGhzProtocolCameTwee* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolCameTwee instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_came_twee_to_load_protocol(SubGhzProtocolCameTwee* instance, void* context);