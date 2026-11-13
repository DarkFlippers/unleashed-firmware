#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_BELKORN_NAME "BelKorn"

typedef struct SubGhzProtocolDecoderBelKorn SubGhzProtocolDecoderBelKorn;
typedef struct SubGhzProtocolEncoderBelKorn SubGhzProtocolEncoderBelKorn;

extern const SubGhzProtocolDecoder subghz_protocol_belkorn_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_belkorn_encoder;
extern const SubGhzProtocol subghz_protocol_belkorn;

/**
 * Allocate SubGhzProtocolEncoderBelKorn.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderBelKorn* pointer to a SubGhzProtocolEncoderBelKorn instance
 */
void* subghz_protocol_encoder_belkorn_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderBelKorn instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_belkorn_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderBelKorn.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderBelKorn* pointer to a SubGhzProtocolDecoderBelKorn instance
 */
void* subghz_protocol_decoder_belkorn_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderBelKorn instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_belkorn_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderBelKorn instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_belkorn_get_hash_data(void* context);

/**
 * Deserialize data SubGhzProtocolDecoderBelKorn.
 * @param context Pointer to a SubGhzProtocolDecoderBelKorn instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_belkorn_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderBelKorn instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_belkorn_get_string(void* context, FuriString* output);
