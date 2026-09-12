#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_CAME_TWEE_NAME "CAME TWEE"

typedef struct SubGhzProtocolDecoderCameTwee SubGhzProtocolDecoderCameTwee;
typedef struct SubGhzProtocolEncoderCameTwee SubGhzProtocolEncoderCameTwee;

extern const SubGhzProtocolDecoder subghz_protocol_came_twee_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_came_twee_encoder;
extern const SubGhzProtocol subghz_protocol_came_twee;

/**
 * Allocate SubGhzProtocolEncoderCameTwee.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderCameTwee* pointer to a SubGhzProtocolEncoderCameTwee instance
 */
void* subghz_protocol_encoder_came_twee_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderCameTwee instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_came_twee_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderCameTwee instance
 */
void subghz_protocol_encoder_came_twee_stop(void* context);

/**
 * Allocate SubGhzProtocolDecoderCameTwee.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderCameTwee* pointer to a SubGhzProtocolDecoderCameTwee instance
 */
void* subghz_protocol_decoder_came_twee_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderCameTwee.
 * @param context Pointer to a SubGhzProtocolDecoderCameTwee instance
 */
void subghz_protocol_decoder_came_twee_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderCameTwee instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_came_twee_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderCameTwee.
 * @param context Pointer to a SubGhzProtocolDecoderCameTwee instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_came_twee_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderCameTwee instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_came_twee_get_string(void* context, FuriString* output);
