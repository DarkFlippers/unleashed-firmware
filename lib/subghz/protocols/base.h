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

/**
 * Set a callback upon completion of successful decoding of one of the protocols.
 * @param decoder_base Pointer to a SubGhzProtocolDecoderBase instance
 * @param callback Callback, SubGhzProtocolDecoderBaseRxCallback
 * @param context Context
 */
void subghz_protocol_decoder_base_set_decoder_callback(
    SubGhzProtocolDecoderBase* decoder_base,
    SubGhzProtocolDecoderBaseRxCallback callback,
    void* context);

/**
 * Getting a textual representation of the received data.
 * @param decoder_base Pointer to a SubGhzProtocolDecoderBase instance
 * @param output Resulting text
 */
bool subghz_protocol_decoder_base_get_string(
    SubGhzProtocolDecoderBase* decoder_base,
    string_t output);

/**
 * Serialize data SubGhzProtocolDecoderBase.
 * @param decoder_base Pointer to a SubGhzProtocolDecoderBase instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_base_serialize(
    SubGhzProtocolDecoderBase* decoder_base,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderBase.
 * @param decoder_base Pointer to a SubGhzProtocolDecoderBase instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_base_deserialize(
    SubGhzProtocolDecoderBase* decoder_base,
    FlipperFormat* flipper_format);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param decoder_base Pointer to a SubGhzProtocolDecoderBase instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_base_get_hash_data(SubGhzProtocolDecoderBase* decoder_base);

// Encoder Base
typedef struct SubGhzProtocolEncoderBase SubGhzProtocolEncoderBase;

struct SubGhzProtocolEncoderBase {
    // Decoder general section
    const SubGhzProtocol* protocol;

    // Callback section
};
