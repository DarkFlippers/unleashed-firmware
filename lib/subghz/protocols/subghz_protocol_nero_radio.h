#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNeroRadio SubGhzProtocolNeroRadio;

/** Allocate SubGhzProtocolNeroRadio
 * 
 * @return SubGhzProtocolNeroRadio* 
 */
SubGhzProtocolNeroRadio* subghz_protocol_nero_radio_alloc();

/** Free SubGhzProtocolNeroRadio
 * 
 * @param instance 
 */
void subghz_protocol_nero_radio_free(SubGhzProtocolNeroRadio* instance);

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolNeroRadio instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_nero_radio_send_key(
    SubGhzProtocolNeroRadio* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolNeroRadio instance
 */
void subghz_protocol_nero_radio_reset(SubGhzProtocolNeroRadio* instance);

/** Analysis of received data
 * 
 * @param instance SubGhzProtocolNeroRadio instance
 */
void subghz_protocol_nero_radio_check_remote_controller(SubGhzProtocolNeroRadio* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolNeroRadio instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_nero_radio_parse(
    SubGhzProtocolNeroRadio* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolNeroRadio* instance
 * @param output   - output string
 */
void subghz_protocol_nero_radio_to_str(SubGhzProtocolNeroRadio* instance, string_t output);

/** Get a string to save the protocol
 * 
 * @param instance  - SubGhzProtocolNeroRadio instance
 * @param output    - the resulting string
 */
void subghz_protocol_nero_radio_to_save_str(SubGhzProtocolNeroRadio* instance, string_t output);

/** Loading protocol from file
 * 
 * @param file_worker - FileWorker file_worker
 * @param instance - SubGhzProtocolNeroRadio instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_nero_radio_to_load_protocol_from_file(
    FileWorker* file_worker,
    SubGhzProtocolNeroRadio* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolNeroRadio instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_nero_radio_to_load_protocol(SubGhzProtocolNeroRadio* instance, void* context);