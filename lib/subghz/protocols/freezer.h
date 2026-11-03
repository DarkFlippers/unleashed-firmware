#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FREEZER_NAME "Freezer"

typedef struct SubGhzProtocolDecoderFreezer SubGhzProtocolDecoderFreezer;
typedef struct SubGhzProtocolEncoderFreezer SubGhzProtocolEncoderFreezer;

extern const SubGhzProtocolDecoder subghz_protocol_freezer_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_freezer_encoder;
extern const SubGhzProtocol subghz_protocol_freezer;

/**
 * Allocate SubGhzProtocolEncoderFreezer.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderFreezer* pointer to a SubGhzProtocolEncoderFreezer instance
 */
void* subghz_protocol_encoder_freezer_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderFreezer instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_freezer_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderFreezer.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderFreezer* pointer to a SubGhzProtocolDecoderFreezer instance
 */
void* subghz_protocol_decoder_freezer_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderFreezer.
 * @param context Pointer to a SubGhzProtocolDecoderFreezer instance
 */
void subghz_protocol_decoder_freezer_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderFreezer instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_freezer_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderFreezer.
 * @param context Pointer to a SubGhzProtocolDecoderFreezer instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_freezer_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFreezer instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_freezer_get_string(void* context, FuriString* output);
