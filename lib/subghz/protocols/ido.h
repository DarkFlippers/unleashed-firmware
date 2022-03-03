#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_IDO_NAME "iDo 117/111"

typedef struct SubGhzProtocolDecoderIDo SubGhzProtocolDecoderIDo;
typedef struct SubGhzProtocolEncoderIDo SubGhzProtocolEncoderIDo;

extern const SubGhzProtocolDecoder subghz_protocol_ido_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_ido_encoder;
extern const SubGhzProtocol subghz_protocol_ido;

void* subghz_protocol_decoder_ido_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_ido_free(void* context);
void subghz_protocol_decoder_ido_reset(void* context);
void subghz_protocol_decoder_ido_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_ido_get_hash_data(void* context);
bool subghz_protocol_decoder_ido_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_decoder_ido_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_ido_get_string(void* context, string_t output);