#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_PROTECO_NAME "Proteco"

typedef struct SubGhzProtocolDecoderProteco SubGhzProtocolDecoderProteco;
typedef struct SubGhzProtocolEncoderProteco SubGhzProtocolEncoderProteco;

extern const SubGhzProtocolDecoder subghz_protocol_proteco_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_proteco_encoder;
extern const SubGhzProtocol subghz_protocol_proteco;

/**
 * Allocate SubGhzProtocolEncoderProteco.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderProteco* pointer to a SubGhzProtocolEncoderProteco instance
 */
void* subghz_protocol_encoder_proteco_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderProteco instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_proteco_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderProteco.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderProteco* pointer to a SubGhzProtocolDecoderProteco instance
 */
void* subghz_protocol_decoder_proteco_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderProteco instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_proteco_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderProteco.
 * @param context Pointer to a SubGhzProtocolDecoderProteco instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_proteco_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderProteco instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_proteco_get_string(void* context, FuriString* output);
