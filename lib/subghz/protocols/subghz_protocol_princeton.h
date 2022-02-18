#pragma once

#include "subghz_protocol_common.h"

struct SubGhzDecoderPrinceton {
    SubGhzProtocolCommon common;
    uint32_t te;
};

/** SubGhzEncoderPrinceton anonymous type */
typedef struct SubGhzEncoderPrinceton SubGhzEncoderPrinceton;

/** Allocate SubGhzEncoderPrinceton
 * @return pointer to SubGhzEncoderPrinceton instance
 */
SubGhzEncoderPrinceton* subghz_encoder_princeton_alloc();

/** Free SubGhzEncoderPrinceton instance
 * @param instance - SubGhzEncoderPrinceton instance
 */
void subghz_encoder_princeton_free(SubGhzEncoderPrinceton* instance);

void subghz_encoder_princeton_stop(SubGhzEncoderPrinceton* instance, uint32_t time_stop);

/** Set new encoder params
 * @param instance - SubGhzEncoderPrinceton instance
 * @param key - 24bit key
 * @param repeat - how many times to repeat 
 * @param frequency - frequency
 */
void subghz_encoder_princeton_set(
    SubGhzEncoderPrinceton* instance,
    uint32_t key,
    size_t repeat,
    uint32_t frequency);

/** Get repeat count left
 * @param instance - SubGhzEncoderPrinceton instance
 * @return repeat count left
 */
size_t subghz_encoder_princeton_get_repeat_left(SubGhzEncoderPrinceton* instance);

/** Print encoder log
 * @param instance - SubGhzEncoderPrinceton instance
 */
void subghz_encoder_princeton_print_log(void* context);

/** Get level duration
 * @param instance - SubGhzEncoderPrinceton instance
 * @return level duration
 */
LevelDuration subghz_encoder_princeton_yield(void* context);

/** SubGhzDecoderPrinceton anonymous type */
typedef struct SubGhzDecoderPrinceton SubGhzDecoderPrinceton;

void subghz_encoder_princeton_set_te(SubGhzEncoderPrinceton* instance, void* decoder);

/** Allocate SubGhzDecoderPrinceton
 * 
 * @return SubGhzDecoderPrinceton* 
 */
SubGhzDecoderPrinceton* subghz_decoder_princeton_alloc();

/** Free SubGhzDecoderPrinceton
 * 
 * @param instance 
 */
void subghz_decoder_princeton_free(SubGhzDecoderPrinceton* instance);

/** Get Te interval protocol
 * 
 * @param context - SubGhzDecoderPrinceton context
 * @return Te interval (us)
 */
uint16_t subghz_protocol_princeton_get_te(void* context);

/** Get upload protocol
 * 
 * @param instance - SubGhzDecoderPrinceton instance
 * @param encoder - SubGhzProtocolCommonEncoder encoder
 * @return bool
 */
bool subghz_protocol_princeton_send_key(
    SubGhzDecoderPrinceton* instance,
    SubGhzProtocolCommonEncoder* encoder);

/** Reset internal state
 * @param instance - SubGhzDecoderPrinceton instance
 */
void subghz_decoder_princeton_reset(SubGhzDecoderPrinceton* instance);

/** Parse accepted duration
 * 
 * @param instance - SubGhzDecoderPrinceton instance
 * @param data - LevelDuration level_duration
 */
void subghz_decoder_princeton_parse(
    SubGhzDecoderPrinceton* instance,
    bool level,
    uint32_t duration);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzDecoderPrinceton* instance
 * @param output   - output string
 */
void subghz_decoder_princeton_to_str(SubGhzDecoderPrinceton* instance, string_t output);

/** Adding data to a file
 * 
 * @param instance  - SubGhzDecoderPrinceton instance
 * @param flipper_format - FlipperFormat 
 * @return bool
 */
bool subghz_decoder_princeton_to_save_file(
    SubGhzDecoderPrinceton* instance,
    FlipperFormat* flipper_format);

/** Loading protocol from file
 * 
 * @param flipper_format - FlipperFormat
 * @param instance - SubGhzDecoderPrinceton instance
 * @param file_path - file path
 * @return bool
 */
bool subghz_decoder_princeton_to_load_protocol_from_file(
    FlipperFormat* flipper_format,
    SubGhzDecoderPrinceton* instance,
    const char* file_path);

/** Loading protocol from bin data
 * 
 * @param instance - SubGhzDecoderPrinceton instance
 * @param context - SubGhzProtocolCommonLoad context
 */
void subghz_decoder_princeton_to_load_protocol(SubGhzDecoderPrinceton* instance, void* context);
