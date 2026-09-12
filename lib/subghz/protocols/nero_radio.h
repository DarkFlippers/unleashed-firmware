#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NERO_RADIO_NAME "Nero Radio"

typedef struct SubGhzProtocolDecoderNeroRadio SubGhzProtocolDecoderNeroRadio;
typedef struct SubGhzProtocolEncoderNeroRadio SubGhzProtocolEncoderNeroRadio;

extern const SubGhzProtocolDecoder subghz_protocol_nero_radio_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nero_radio_encoder;
extern const SubGhzProtocol subghz_protocol_nero_radio;

/**
 * Allocate SubGhzProtocolEncoderNeroRadio.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderNeroRadio* pointer to a SubGhzProtocolEncoderNeroRadio instance
 */
void* subghz_protocol_encoder_nero_radio_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderNeroRadio instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderNeroRadio.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderNeroRadio* pointer to a SubGhzProtocolDecoderNeroRadio instance
 */
void* subghz_protocol_decoder_nero_radio_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderNeroRadio instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_nero_radio_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderNeroRadio.
 * @param context Pointer to a SubGhzProtocolDecoderNeroRadio instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_nero_radio_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNeroRadio instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nero_radio_get_string(void* context, FuriString* output);
