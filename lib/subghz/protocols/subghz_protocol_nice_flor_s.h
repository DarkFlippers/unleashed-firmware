#pragma once

#include "subghz_protocol_common.h"

typedef struct SubGhzProtocolNiceFlorS SubGhzProtocolNiceFlorS;

/** Allocate SubGhzProtocolNiceFlorS
 * 
 * @return SubGhzProtocolNiceFlorS* 
 */
SubGhzProtocolNiceFlorS* subghz_protocol_nice_flor_s_alloc();

/** Free SubGhzProtocolNiceFlorS
 * 
 * @param instance 
 */
void subghz_protocol_nice_flor_s_free(SubGhzProtocolNiceFlorS* instance);

/** File name rainbow table Nice Flor-S
 * 
 * @param instance - SubGhzProtocolNiceFlorS instance
 * @param file_name - "path/file_name"
 */
void subghz_protocol_nice_flor_s_name_file(SubGhzProtocolNiceFlorS* instance, const char* name);

/** Sends the key on the air
 * 
 * @param instance - SubGhzProtocolNiceFlorS instance
 * @param key - key send
 * @param bit - count bit key
 * @param repeat - repeat send key
 */
void subghz_protocol_nice_flor_s_send_key(
    SubGhzProtocolNiceFlorS* instance,
    uint64_t key,
    uint8_t bit,
    uint8_t repeat);

/** Reset internal state
 * @param instance - SubGhzProtocolNiceFlorS instance
 */
void subghz_protocol_nice_flor_s_reset(SubGhzProtocolNiceFlorS* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolNiceFlorS instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_nice_flor_s_parse(
    SubGhzProtocolNiceFlorS* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolNiceFlorS* instance
 * @param output   - output string
 */
void subghz_protocol_nice_flor_s_to_str(SubGhzProtocolNiceFlorS* instance, string_t output);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzProtocolNiceFlorS instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_nice_flor_s_to_load_protocol(SubGhzProtocolNiceFlorS* instance, void* context);