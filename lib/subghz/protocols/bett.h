#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_BETT_NAME "BETT"

typedef struct SubGhzProtocolDecoderBETT SubGhzProtocolDecoderBETT;
typedef struct SubGhzProtocolEncoderBETT SubGhzProtocolEncoderBETT;

extern const SubGhzProtocolDecoder subghz_protocol_bett_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_bett_encoder;
extern const SubGhzProtocol subghz_protocol_bett;

/**
 * Allocate SubGhzProtocolEncoderBETT.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderBETT* pointer to a SubGhzProtocolEncoderBETT instance
 */
void* subghz_protocol_encoder_bett_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderBETT instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_bett_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderBETT.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderBETT* pointer to a SubGhzProtocolDecoderBETT instance
 */
void* subghz_protocol_decoder_bett_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_bett_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderBETT.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_bett_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_bett_get_string(void* context, FuriString* output);
