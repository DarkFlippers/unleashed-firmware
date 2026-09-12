#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HORMANN_HSM_NAME "Hormann HSM"

typedef struct SubGhzProtocolDecoderHormann SubGhzProtocolDecoderHormann;
typedef struct SubGhzProtocolEncoderHormann SubGhzProtocolEncoderHormann;

extern const SubGhzProtocolDecoder subghz_protocol_hormann_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_hormann_encoder;
extern const SubGhzProtocol subghz_protocol_hormann;

/**
 * Allocate SubGhzProtocolEncoderHormann.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHormann* pointer to a SubGhzProtocolEncoderHormann instance
 */
void* subghz_protocol_encoder_hormann_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHormann instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_hormann_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderHormann instance
 */
void subghz_protocol_encoder_hormann_stop(void* context);

/**
 * Allocate SubGhzProtocolDecoderHormann.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHormann* pointer to a SubGhzProtocolDecoderHormann instance
 */
void* subghz_protocol_decoder_hormann_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHormann instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_hormann_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderHormann.
 * @param context Pointer to a SubGhzProtocolDecoderHormann instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_hormann_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHormann instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_hormann_get_string(void* context, FuriString* output);
