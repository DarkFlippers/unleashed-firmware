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
 * @param instance - SubGhzProtocolNeroSketch instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_nero_sketch_send_key(
    SubGhzProtocolNeroSketch* instance,
    SubGhzProtocolCommonEncoder* encoder);

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
void subghz_protocol_nero_sketch_parse(
    SubGhzProtocolNeroSketch* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolNeroSketch* instance
 * @param output   - output string
 */
void subghz_protocol_nero_sketch_to_str(SubGhzProtocolNeroSketch* instance, string_t output);

/** Adding data to a file
 * 
 * @param instance  - SubGhzProtocolNeroSketch instance
 * @param flipper_format - FlipperFormat 
 * @return bool
 */
bool subghz_protocol_nero_sketch_to_save_file(
    SubGhzProtocolNeroSketch* instance,
    FlipperFormat* flipper_format);

/** Loading protocol from file
 * 
 * @param flipper_format - FlipperFormat
 * @param instance - SubGhzProtocolNeroSketch instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_nero_sketch_to_load_protocol_from_file(
    FlipperFormat* flipper_format,
    SubGhzProtocolNeroSketch* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolNeroSketch instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_nero_sketch_to_load_protocol(SubGhzProtocolNeroSketch* instance, void* context);
