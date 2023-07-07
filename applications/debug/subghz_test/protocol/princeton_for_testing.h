#pragma once

//#include "base.h"
#include <furi.h>
#include <lib/toolbox/level_duration.h>

/** SubGhzDecoderPrinceton anonymous type */
typedef struct SubGhzDecoderPrinceton SubGhzDecoderPrinceton;
/** SubGhzEncoderPrinceton anonymous type */
typedef struct SubGhzEncoderPrinceton SubGhzEncoderPrinceton;

typedef void (*SubGhzDecoderPrincetonCallback)(SubGhzDecoderPrinceton* parser, void* context);

/** 
 * Allocate SubGhzEncoderPrinceton
 * @return pointer to SubGhzEncoderPrinceton instance
 */
SubGhzEncoderPrinceton* subghz_encoder_princeton_for_testing_alloc();

/** 
 * Free SubGhzEncoderPrinceton instance
 * @param instance - SubGhzEncoderPrinceton instance
 */
void subghz_encoder_princeton_for_testing_free(SubGhzEncoderPrinceton* instance);

/**
 * Forced transmission stop.
 * @param instance Pointer to a SubGhzEncoderPrinceton instance
 * @param time_stop Transmission stop time, ms
 */
void subghz_encoder_princeton_for_testing_stop(
    SubGhzEncoderPrinceton* instance,
    uint32_t time_stop);

/** 
 * Set new encoder params
 * @param instance - SubGhzEncoderPrinceton instance
 * @param key - 24bit key
 * @param repeat - how many times to repeat 
 * @param frequency - frequency
 */
void subghz_encoder_princeton_for_testing_set(
    SubGhzEncoderPrinceton* instance,
    uint32_t key,
    size_t repeat,
    uint32_t frequency);

/** 
 * Get repeat count left
 * @param instance - SubGhzEncoderPrinceton instance
 * @return repeat count left
 */
size_t subghz_encoder_princeton_for_testing_get_repeat_left(SubGhzEncoderPrinceton* instance);

/** 
 * Print encoder log
 * @param instance - SubGhzEncoderPrinceton instance
 */
void subghz_encoder_princeton_for_testing_print_log(void* context);

/** 
 * Get level duration
 * @param instance - SubGhzEncoderPrinceton instance
 * @return level duration
 */
LevelDuration subghz_encoder_princeton_for_testing_yield(void* context);

/** 
 * Allocate SubGhzDecoderPrinceton
 * @return SubGhzDecoderPrinceton* 
 */
SubGhzDecoderPrinceton* subghz_decoder_princeton_for_testing_alloc();

/** 
 * Free SubGhzDecoderPrinceton
 * @param instance 
 */
void subghz_decoder_princeton_for_testing_free(SubGhzDecoderPrinceton* instance);

void subghz_decoder_princeton_for_testing_set_callback(
    SubGhzDecoderPrinceton* instance,
    SubGhzDecoderPrincetonCallback callback,
    void* context);

/** 
 * Reset internal state
 * @param instance - SubGhzDecoderPrinceton instance
 */
void subghz_decoder_princeton_for_testing_reset(SubGhzDecoderPrinceton* instance);

/** 
 * Parse accepted duration
 * @param instance - SubGhzDecoderPrinceton instance
 * @param data - LevelDuration level_duration
 */
void subghz_decoder_princeton_for_testing_parse(
    SubGhzDecoderPrinceton* instance,
    bool level,
    uint32_t duration);
