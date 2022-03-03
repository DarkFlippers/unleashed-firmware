#pragma once

#include "../types.h"

typedef struct SubGhzProtocolDecoderBase SubGhzProtocolDecoderBase;

typedef void (
    *SubGhzProtocolDecoderBaseRxCallback)(SubGhzProtocolDecoderBase* instance, void* context);

typedef void (
    *SubGhzProtocolDecoderBaseSerialize)(SubGhzProtocolDecoderBase* decoder_base, string_t output);

struct SubGhzProtocolDecoderBase {
    // Decoder general section
    const SubGhzProtocol* protocol;

    // Callback section
    SubGhzProtocolDecoderBaseRxCallback callback;
    void* context;
};

void subghz_protocol_decoder_base_set_decoder_callback(
    SubGhzProtocolDecoderBase* decoder_base,
    SubGhzProtocolDecoderBaseRxCallback callback,
    void* context);

bool subghz_protocol_decoder_base_get_string(
    SubGhzProtocolDecoderBase* decoder_base,
    string_t output);

bool subghz_protocol_decoder_base_serialize(
    SubGhzProtocolDecoderBase* decoder_base,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

bool subghz_protocol_decoder_base_deserialize(
    SubGhzProtocolDecoderBase* decoder_base,
    FlipperFormat* flipper_format);

uint8_t subghz_protocol_decoder_base_get_hash_data(SubGhzProtocolDecoderBase* decoder_base);

// Encoder Base
typedef struct SubGhzProtocolEncoderBase SubGhzProtocolEncoderBase;

struct SubGhzProtocolEncoderBase {
    // Decoder general section
    const SubGhzProtocol* protocol;

    // Callback section
};
