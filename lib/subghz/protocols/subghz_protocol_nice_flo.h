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
 * @param instance - SubGhzProtocolCame instance
 * @param encoder - SubGhzProtocolEncoderCommon encoder
 * @return bool
 */
bool subghz_protocol_nice_flo_send_key(SubGhzProtocolNiceFlo* instance, SubGhzProtocolEncoderCommon* encoder);

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

void subghz_protocol_nice_flo_to_save_str(SubGhzProtocolNiceFlo* instance, string_t output);
bool subghz_protocol_nice_flo_to_load_protocol(FileWorker* file_worker, SubGhzProtocolNiceFlo* instance);
