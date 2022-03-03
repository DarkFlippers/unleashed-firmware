#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_KEELOQ_NAME "KeeLoq"

typedef struct SubGhzProtocolDecoderKeeloq SubGhzProtocolDecoderKeeloq;
typedef struct SubGhzProtocolEncoderKeeloq SubGhzProtocolEncoderKeeloq;

extern const SubGhzProtocolDecoder subghz_protocol_keeloq_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_keeloq_encoder;
extern const SubGhzProtocol subghz_protocol_keeloq;

void* subghz_protocol_encoder_keeloq_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_keeloq_free(void* context);
bool subghz_protocol_keeloq_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_encoder_keeloq_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_encoder_keeloq_stop(void* context);
LevelDuration subghz_protocol_encoder_keeloq_yield(void* context);
void* subghz_protocol_decoder_keeloq_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_keeloq_free(void* context);
void subghz_protocol_decoder_keeloq_reset(void* context);
void subghz_protocol_decoder_keeloq_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_keeloq_get_hash_data(void* context);
bool subghz_protocol_decoder_keeloq_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_decoder_keeloq_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_keeloq_get_string(void* context, string_t output);
