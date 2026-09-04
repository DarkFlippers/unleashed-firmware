#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_GALO_C02_NAME "GALO C02"

typedef struct SubGhzProtocolDecoderGaloC02 SubGhzProtocolDecoderGaloC02;
typedef struct SubGhzProtocolEncoderGaloC02 SubGhzProtocolEncoderGaloC02;

extern const SubGhzProtocolDecoder subghz_protocol_galo_c02_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_galo_c02_encoder;
extern const SubGhzProtocol subghz_protocol_galo_c02;

void* subghz_protocol_encoder_galo_c02_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_galo_c02_free(void* context);
SubGhzProtocolStatus
    subghz_protocol_encoder_galo_c02_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_encoder_galo_c02_stop(void* context);
LevelDuration subghz_protocol_encoder_galo_c02_yield(void* context);

void* subghz_protocol_decoder_galo_c02_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_galo_c02_free(void* context);
void subghz_protocol_decoder_galo_c02_reset(void* context);
void subghz_protocol_decoder_galo_c02_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_galo_c02_get_hash_data(void* context);
SubGhzProtocolStatus subghz_protocol_decoder_galo_c02_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);
SubGhzProtocolStatus
    subghz_protocol_decoder_galo_c02_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_galo_c02_get_string(void* context, FuriString* output);
