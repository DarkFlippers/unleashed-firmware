#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FAAC_TM_NAME "Faac TM"

typedef struct SubGhzProtocolDecoderFaacTm SubGhzProtocolDecoderFaacTm;
typedef struct SubGhzProtocolEncoderFaacTm SubGhzProtocolEncoderFaacTm;

extern const SubGhzProtocolDecoder subghz_protocol_faac_tm_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_faac_tm_encoder;
extern const SubGhzProtocol subghz_protocol_faac_tm;

/**
 * Allocate SubGhzProtocolEncoderFaacTm.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderFaacTm* pointer to a SubGhzProtocolEncoderFaacTm instance
 */
void* subghz_protocol_encoder_faac_tm_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderFaacTm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_faac_tm_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderFaacTm.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderFaacTm* pointer to a SubGhzProtocolDecoderFaacTm instance
 */
void* subghz_protocol_decoder_faac_tm_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderFaacTm instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_faac_tm_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderFaacTm.
 * @param context Pointer to a SubGhzProtocolDecoderFaacTm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_faac_tm_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFaacTm instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_faac_tm_get_string(void* context, FuriString* output);
