#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_CAME_NAME "CAME"

typedef struct SubGhzProtocolDecoderCame SubGhzProtocolDecoderCame;
typedef struct SubGhzProtocolEncoderCame SubGhzProtocolEncoderCame;

extern const SubGhzProtocolDecoder subghz_protocol_came_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_came_encoder;
extern const SubGhzProtocol subghz_protocol_came;

void* subghz_protocol_encoder_came_alloc(SubGhzEnvironment* environment);

void subghz_protocol_encoder_came_free(void* context);

bool subghz_protocol_encoder_came_deserialize(void* context, FlipperFormat* flipper_format);

void subghz_protocol_encoder_came_stop(void* context);

LevelDuration subghz_protocol_encoder_came_yield(void* context);

/** Allocate SubGhzProtocolCame
 * 
 * @return SubGhzProtocolCame* 
 */
void* subghz_protocol_decoder_came_alloc(SubGhzEnvironment* environment);

/** Free SubGhzProtocolCame
 * 
 * @param instance 
 */
void subghz_protocol_decoder_came_free(void* context);

/** Reset internal state
 * @param instance - SubGhzProtocolCame instance
 */
void subghz_protocol_decoder_came_reset(void* context);

/** Parse accepted duration
 * 
 * @param instance - SubGhzProtocolCame instance
 * @param data - LevelDuration level_duration
 */
void subghz_protocol_decoder_came_feed(void* context, bool level, uint32_t duration);

uint8_t subghz_protocol_decoder_came_get_hash_data(void* context);

/** Outputting information from the parser
 * 
 * @param instance - SubGhzProtocolCame* instance
 * @param output   - output string
 */
bool subghz_protocol_decoder_came_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

bool subghz_protocol_decoder_came_deserialize(void* context, FlipperFormat* flipper_format);

void subghz_protocol_decoder_came_get_string(void* context, string_t output);
