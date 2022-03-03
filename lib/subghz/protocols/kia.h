#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_KIA_NAME "KIA Seed"

typedef struct SubGhzProtocolDecoderKIA SubGhzProtocolDecoderKIA;
typedef struct SubGhzProtocolEncoderKIA SubGhzProtocolEncoderKIA;

extern const SubGhzProtocolDecoder subghz_protocol_kia_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_kia_encoder;
extern const SubGhzProtocol subghz_protocol_kia;

void* subghz_protocol_decoder_kia_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_kia_free(void* context);
void subghz_protocol_decoder_kia_reset(void* context);
void subghz_protocol_decoder_kia_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_kia_get_hash_data(void* context);
bool subghz_protocol_decoder_kia_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_decoder_kia_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_kia_get_string(void* context, string_t output);
