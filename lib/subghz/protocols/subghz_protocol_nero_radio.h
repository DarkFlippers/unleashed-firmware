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

/** Adding data to a file
 * 
 * @param instance  - SubGhzProtocolNeroRadio instance
 * @param flipper_format - FlipperFormat 
 * @return bool
 */
bool subghz_protocol_nero_radio_to_save_file(
    SubGhzProtocolNeroRadio* instance,
    FlipperFormat* flipper_format);

/** Loading protocol from file
 * 
 * @param flipper_format - FlipperFormat
 * @param instance - SubGhzProtocolNeroRadio instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_nero_radio_to_load_protocol_from_file(
    FlipperFormat* flipper_format,
    SubGhzProtocolNeroRadio* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolNeroRadio instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_nero_radio_to_load_protocol(SubGhzProtocolNeroRadio* instance, void* context);