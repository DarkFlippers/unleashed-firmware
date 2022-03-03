#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_PRINCETON_NAME "Princeton"

typedef struct SubGhzProtocolDecoderPrinceton SubGhzProtocolDecoderPrinceton;
typedef struct SubGhzProtocolEncoderPrinceton SubGhzProtocolEncoderPrinceton;

extern const SubGhzProtocolDecoder subghz_protocol_princeton_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_princeton_encoder;
extern const SubGhzProtocol subghz_protocol_princeton;

void* subghz_protocol_encoder_princeton_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_princeton_free(void* context);
bool subghz_protocol_encoder_princeton_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_encoder_princeton_stop(void* context);
LevelDuration subghz_protocol_encoder_princeton_yield(void* context);
void* subghz_protocol_decoder_princeton_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_princeton_free(void* context);
void subghz_protocol_decoder_princeton_reset(void* context);
void subghz_protocol_decoder_princeton_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_princeton_get_hash_data(void* context);
bool subghz_protocol_decoder_princeton_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_decoder_princeton_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_princeton_get_string(void* context, string_t output);
