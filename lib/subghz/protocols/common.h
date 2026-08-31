#pragma once

#include <stddef.h>

#include "base.h"

#include "../blocks/decoder.h"
#include "../blocks/encoder.h"
#include "../blocks/generic.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Shared implementations of the protocol vtable entries that are identical across protocols.
 * They reach the instance through a void* the same way the per-protocol copies did, and only
 * touch members of one of the leading sequences below, so any protocol whose struct starts with
 * those members can point its vtable straight at them; the SUBGHZ_ASSERT_*_LAYOUT macros hold
 * that down. They log through generic.protocol_name, which every alloc must therefore set.
 * Protocols needing extra work in a slot either keep their own copy, or call the shared helper
 * and then write their own fields the way princeton does.
 *
 * subghz_protocol_common_append_data_2 is the exception: a plain helper rather than a slot,
 * shared by the serialize slots and the encoder create_data paths.
 */

typedef struct {
    SubGhzProtocolEncoderBase base;
    SubGhzProtocolBlockEncoder encoder;
} SubGhzProtocolEncoderCommon;

typedef struct {
    SubGhzProtocolEncoderCommon common;
    SubGhzBlockGeneric generic;
} SubGhzProtocolEncoderCommonGeneric;

typedef struct {
    SubGhzProtocolDecoderBase base;
    SubGhzBlockDecoder decoder;
    SubGhzBlockGeneric generic;
} SubGhzProtocolDecoderCommon;

typedef struct {
    SubGhzProtocolDecoderCommon common;
    uint32_t te;
} SubGhzProtocolDecoderCommonTe;

/**
 * The shared handlers reach members by offset, so a protocol that reorders its prefix would
 * silently alias the wrong one. Pinning te alone is not enough: decoder and generic are both
 * multiples of 8, so swapping them leaves te at the same offset.
 */
#define SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(type)                                        \
    _Static_assert(                                                                      \
        offsetof(type, base) == offsetof(SubGhzProtocolDecoderCommon, base) &&           \
            offsetof(type, decoder) == offsetof(SubGhzProtocolDecoderCommon, decoder) && \
            offsetof(type, generic) == offsetof(SubGhzProtocolDecoderCommon, generic),   \
        #type " must start with the SubGhzProtocolDecoderCommon member sequence")

#define SUBGHZ_ASSERT_ENCODER_COMMON_LAYOUT(type)                                      \
    _Static_assert(                                                                    \
        offsetof(type, base) == offsetof(SubGhzProtocolEncoderCommon, base) &&         \
            offsetof(type, encoder) == offsetof(SubGhzProtocolEncoderCommon, encoder), \
        #type " must start with the SubGhzProtocolEncoderCommon member sequence")

#define SUBGHZ_ASSERT_ENCODER_GENERIC_LAYOUT(type)                                        \
    SUBGHZ_ASSERT_ENCODER_COMMON_LAYOUT(type);                                            \
    _Static_assert(                                                                       \
        offsetof(type, generic) == offsetof(SubGhzProtocolEncoderCommonGeneric, generic), \
        #type " must keep generic directly after encoder")

#define SUBGHZ_ASSERT_DECODER_TE_LAYOUT(type)                              \
    SUBGHZ_ASSERT_DECODER_COMMON_LAYOUT(type);                             \
    _Static_assert(                                                        \
        offsetof(type, te) == offsetof(SubGhzProtocolDecoderCommonTe, te), \
        #type " must keep te directly after generic")

/**
 * Allocate a decoder instance and bind it to its protocol.
 * @param instance_size sizeof the protocol's decoder struct
 * @param protocol Pointer to the SubGhzProtocol the instance decodes
 * @return Pointer to the new instance
 */
void* subghz_protocol_decoder_common_alloc(size_t instance_size, const SubGhzProtocol* protocol);

/**
 * Allocate an encoder instance, bind it to its protocol and size its upload buffer.
 * @param instance_size sizeof the protocol's encoder struct
 * @param protocol Pointer to the SubGhzProtocol the instance encodes
 * @param repeat Number of times the upload is repeated
 * @param size_upload Upload buffer length, in LevelDuration entries
 * @return Pointer to the new instance
 */
void* subghz_protocol_encoder_common_alloc(
    size_t instance_size,
    const SubGhzProtocol* protocol,
    size_t repeat,
    size_t size_upload);

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

/**
 * Write generic.data_2 as a big-endian "Data" field, carrying a running status.
 * Rewinds flipper_format. Shared by the decoder serialize slots and the encoder
 * create_data paths.
 * @param status Status so far, returned unchanged unless this call itself fails
 * @param generic Pointer to a SubGhzBlockGeneric instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_common_append_data_2(
    SubGhzProtocolStatus status,
    SubGhzBlockGeneric* generic,
    FlipperFormat* flipper_format);

/**
 * Serialize decoder data and append generic.data_2 as a "Data" field.
 * @param context Pointer to a SubGhzProtocolDecoderCommon instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_common_serialize_data_2(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Serialize decoder data and append the TE value.
 * @param context Pointer to a SubGhzProtocolDecoderCommonTe instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_common_serialize_te(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

#ifdef __cplusplus
}
#endif
