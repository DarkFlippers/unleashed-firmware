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

/** Get a string to save the protocol
 * 
 * @param instance  - SubGhzProtocolCameTwee instance
 * @param output    - the resulting string
 */
void subghz_protocol_came_twee_to_save_str(SubGhzProtocolCameTwee* instance, string_t output);

/** Loading protocol from file
 * 
 * @param file_worker - FileWorker file_worker
 * @param instance - SubGhzProtocolCameTwee instance
 * @return bool
 */
bool subghz_protocol_came_twee_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolCameTwee* instance);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolCameTwee instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_came_twee_to_load_protocol(SubGhzProtocolCameTwee* instance, void* context);