#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NORD_ICE_NAME "Nord ICE"

typedef struct SubGhzProtocolDecoderNord_Ice SubGhzProtocolDecoderNord_Ice;
typedef struct SubGhzProtocolEncoderNord_Ice SubGhzProtocolEncoderNord_Ice;

extern const SubGhzProtocolDecoder subghz_protocol_nord_ice_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nord_ice_encoder;
extern const SubGhzProtocol subghz_protocol_nord_ice;

/**
 * Allocate SubGhzProtocolEncoderNord_Ice.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderNord_Ice* pointer to a SubGhzProtocolEncoderNord_Ice instance
 */
void* subghz_protocol_encoder_nord_ice_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderNord_Ice instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_nord_ice_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderNord_Ice.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderNord_Ice* pointer to a SubGhzProtocolDecoderNord_Ice instance
 */
void* subghz_protocol_decoder_nord_ice_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_nord_ice_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderNord_Ice.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_nord_ice_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nord_ice_get_string(void* context, FuriString* output);
