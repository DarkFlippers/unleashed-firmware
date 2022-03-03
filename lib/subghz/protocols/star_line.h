#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_STAR_LINE_NAME "Star Line"

typedef struct SubGhzProtocolDecoderStarLine SubGhzProtocolDecoderStarLine;
typedef struct SubGhzProtocolEncoderStarLine SubGhzProtocolEncoderStarLine;

extern const SubGhzProtocolDecoder subghz_protocol_star_line_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_star_line_encoder;
extern const SubGhzProtocol subghz_protocol_star_line;

void* subghz_protocol_decoder_star_line_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_star_line_free(void* context);
void subghz_protocol_decoder_star_line_reset(void* context);
void subghz_protocol_decoder_star_line_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_star_line_get_hash_data(void* context);
bool subghz_protocol_decoder_star_line_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);
bool subghz_protocol_decoder_star_line_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_star_line_get_string(void* context, string_t output);
