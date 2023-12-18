#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "base.h"
#include "../blocks/generic.h"
#include <lib/subghz/blocks/math.h>
#include <lib/toolbox/manchester_decoder.h>
#include <lib/toolbox/manchester_encoder.h>

#define SUBGHZ_PROTOCOL_HONEYWELL_NAME "Honeywell Sec"

typedef struct SubGhzProtocolDecoderHoneywell SubGhzProtocolDecoderHoneywell;
typedef struct SubGhzProtocolEncoderHoneywell SubGhzProtocolEncoderHoneywell;

extern const SubGhzProtocolDecoder subghz_protocol_honeywell_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_honeywell_encoder;
extern const SubGhzProtocol subghz_protocol_honeywell;

void* subghz_protocol_decoder_honeywell_alloc(SubGhzEnvironment* environment);

void subghz_protocol_decoder_honeywell_free(void* context);

void subghz_protocol_decoder_honeywell_reset(void* context);

void subghz_protocol_decoder_honeywell_feed(void* context, bool level, uint32_t duration);

uint8_t subghz_protocol_decoder_honeywell_get_hash_data(void* context);

SubGhzProtocolStatus subghz_protocol_decoder_honeywell_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

SubGhzProtocolStatus
    subghz_protocol_decoder_honeywell_deserialize(void* context, FlipperFormat* flipper_format);

void subghz_protocol_decoder_honeywell_get_string(void* context, FuriString* output);

static const SubGhzBlockConst subghz_protocol_honeywell_const = {
    .te_long = 280,
    .te_short = 143,
    .te_delta = 51,
    .min_count_bit_for_found = 62,
};

struct SubGhzProtocolDecoderHoneywell {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockGeneric generic;
    SubGhzBlockDecoder decoder;
    ManchesterState manchester_saved_state;
};

struct SubGhzProtocolEncoderHoneywell {
    SubGhzProtocolEncoderBase base;
    SubGhzBlockGeneric generic;
    SubGhzProtocolBlockEncoder encoder;
};
