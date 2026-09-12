#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_ROGER_NAME "Roger"

typedef struct SubGhzProtocolDecoderRoger SubGhzProtocolDecoderRoger;
typedef struct SubGhzProtocolEncoderRoger SubGhzProtocolEncoderRoger;

extern const SubGhzProtocolDecoder subghz_protocol_roger_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_roger_encoder;
extern const SubGhzProtocol subghz_protocol_roger;

/**
 * Allocate SubGhzProtocolEncoderRoger.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderRoger* pointer to a SubGhzProtocolEncoderRoger instance
 */
void* subghz_protocol_encoder_roger_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderRoger instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_roger_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderRoger.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderRoger* pointer to a SubGhzProtocolDecoderRoger instance
 */
void* subghz_protocol_decoder_roger_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderRoger instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_roger_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderRoger.
 * @param context Pointer to a SubGhzProtocolDecoderRoger instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_roger_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderRoger instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_roger_get_string(void* context, FuriString* output);
