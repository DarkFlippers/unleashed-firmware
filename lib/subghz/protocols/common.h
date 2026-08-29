#pragma once

#include "base.h"

#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shared implementations of the protocol vtable entries that are byte-for-byte identical across
 * protocols. They reach the instance through a void* the same way the per-protocol copies did,
 * and only ever touch members of the leading common sequence below, so any protocol whose struct
 * starts with those members can point its vtable straight at them. Protocols that need extra work
 * in one of these slots (extra state to reset, a different teardown) keep their own copy.
 */

typedef struct {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;
} SubGhzProtocolEncoderCommon;

typedef struct {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
} SubGhzProtocolDecoderCommon;

/**
 * Free an encoder instance along with its upload buffer.
 * @param context Pointer to an encoder instance
 */
void subghz_protocol_encoder_common_free(void* context);

/**
 * Forced transmission stop.
 * @param context Pointer to an encoder instance
 */
void subghz_protocol_encoder_common_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to an encoder instance
 * @return LevelDuration
 */
LevelDuration subghz_protocol_encoder_common_yield(void* context);

/**
 * Free a decoder instance.
 * @param context Pointer to a decoder instance
 */
void subghz_protocol_decoder_common_free(void* context);

/**
 * Reset the decoder parser back to its first step.
 * @param context Pointer to a decoder instance
 */
void subghz_protocol_decoder_common_reset(void* context);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a decoder instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_common_get_hash_data(void* context);

/**
 * Serialize decoder data.
 * @param context Pointer to a decoder instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_common_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

#ifdef __cplusplus
}
#endif
