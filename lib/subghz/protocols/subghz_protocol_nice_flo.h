#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNiceFlo SubGhzProtocolNiceFlo;

/** Allocate SubGhzProtocolNiceFlo
 * 
 * @return SubGhzProtocolNiceFlo* 
 */
SubGhzProtocolNiceFlo* subghz_protocol_nice_flo_alloc();

/** Free SubGhzProtocolNiceFlo
 * 
 * @param instance 
 */
void subghz_protocol_nice_flo_free(SubGhzProtocolNiceFlo* instance);

/** Get upload protocol
 * 
 * @param instance - SubGhzProtocolNiceFlo instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_nice_flo_send_key(
    SubGhzProtocolNiceFlo* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzProtocolNiceFlo instance
 */
void subghz_protocol_nice_flo_reset(SubGhzProtocolNiceFlo* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolNiceFlo instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_nice_flo_parse(SubGhzProtocolNiceFlo* instance, bool level, uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolNiceFlo* instance
 * @param output   - output string
 */
void subghz_protocol_nice_flo_to_str(SubGhzProtocolNiceFlo* instance, string_t output);

/** Adding data to a file
 * 
 * @param instance  - SubGhzProtocolNiceFlo instance
 * @param flipper_file - FlipperFile 
 * @return bool
 */
bool subghz_protocol_nice_flo_to_save_file(
    SubGhzProtocolNiceFlo* instance,
    FlipperFile* flipper_file);

/** Loading protocol from file
 * 
 * @param flipper_file - FlipperFile 
 * @param instance - SubGhzProtocolNiceFlo instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_protocol_nice_flo_to_load_protocol_from_file(
    FlipperFile* flipper_file,
    SubGhzProtocolNiceFlo* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolNiceFlo instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_nice_flo_to_load_protocol(SubGhzProtocolNiceFlo* instance, void* context);
