#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NERO_RADIO_NAME "Nero Radio"

typedef struct SubGhzProtocolDecoderNeroRadio SubGhzProtocolDecoderNeroRadio;
typedef struct SubGhzProtocolEncoderNeroRadio SubGhzProtocolEncoderNeroRadio;

extern const SubGhzProtocolDecoder subghz_protocol_nero_radio_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nero_radio_encoder;
extern const SubGhzProtocol subghz_protocol_nero_radio;

void* subghz_protocol_encoder_nero_radio_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_nero_radio_free(void* context);
bool subghz_protocol_encoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_encoder_nero_radio_stop(void* context);
LevelDuration subghz_protocol_encoder_nero_radio_yield(void* context);
void* subghz_protocol_decoder_nero_radio_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_nero_radio_free(void* context);
void subghz_protocol_decoder_nero_radio_reset(void* context);
void subghz_protocol_decoder_nero_radio_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_nero_radio_get_hash_data(void* context);
bool subghz_protocol_decoder_nero_radio_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_decoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_nero_radio_get_string(void* context, string_t output);
