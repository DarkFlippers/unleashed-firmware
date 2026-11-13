#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_CAME_NAME "CAME"

/**
 * Name of the variant a frame length belongs to: the CAME decoder also carries Prastel and
 * Airforce remotes, and the history list has to show what the info screen shows.
 * @param bits Frame length, generic.data_count_bit
 * @return Variant name, or the protocol name when the length is CAME's own
 */
const char* subghz_protocol_came_get_variant_name(uint16_t bits);

typedef struct SubGhzProtocolDecoderCame SubGhzProtocolDecoderCame;
typedef struct SubGhzProtocolEncoderCame SubGhzProtocolEncoderCame;

extern const SubGhzProtocolDecoder subghz_protocol_came_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_came_encoder;
extern const SubGhzProtocol subghz_protocol_came;

/**
 * Allocate SubGhzProtocolEncoderCame.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderCame* pointer to a SubGhzProtocolEncoderCame instance
 */
void* subghz_protocol_encoder_came_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderCame instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_came_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderCame.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderCame* pointer to a SubGhzProtocolDecoderCame instance
 */
void* subghz_protocol_decoder_came_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_came_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderCame.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_came_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderCame instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_came_get_string(void* context, FuriString* output);
