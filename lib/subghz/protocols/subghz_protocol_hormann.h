#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolHormann SubGhzProtocolHormann;

/** Allocate SubGhzProtocolHormann
 * 
 * @return SubGhzProtocolHormann* 
 */
SubGhzProtocolHormann* subghz_protocol_hormann_alloc();

/** Free SubGhzProtocolHormann
 * 
 * @param instance 
 */
void subghz_protocol_hormann_free(SubGhzProtocolHormann* instance);

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolHormann instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_hormann_send_key(
    SubGhzProtocolHormann* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolHormann instance
 */
void subghz_protocol_hormann_reset(SubGhzProtocolHormann* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolHormann instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_hormann_parse(SubGhzProtocolHormann* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolHormann* instance
 * @param output   - output string
 */
void subghz_protocol_hormann_to_str(SubGhzProtocolHormann* instance, string_t output);

/** Get a string to save the protocol
 * 
 * @param instance  - SubGhzProtocolHormann instance
 * @param output    - the resulting string
 */
void subghz_protocol_hormann_to_save_str(SubGhzProtocolHormann* instance, string_t output);

/** Loading protocol from file
 * 
 * @param file_worker - FileWorker file_worker
 * @param instance - SubGhzProtocolHormann instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_hormann_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolHormann* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolHormann instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_hormann_to_load_protocol(SubGhzProtocolHormann* instance, void* context);
