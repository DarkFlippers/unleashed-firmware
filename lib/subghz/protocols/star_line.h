#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_STAR_LINE_NAME "Star Line"

typedef struct SubGhzProtocolDecoderStarLine SubGhzProtocolDecoderStarLine;
typedef struct SubGhzProtocolEncoderStarLine SubGhzProtocolEncoderStarLine;

extern const SubGhzProtocolDecoder subghz_protocol_star_line_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_star_line_encoder;
extern const SubGhzProtocol subghz_protocol_star_line;

/**
 * Allocate SubGhzProtocolDecoderStarLine.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderStarLine* pointer to a SubGhzProtocolDecoderStarLine instance
 */
void* subghz_protocol_decoder_star_line_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderStarLine.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 */
void subghz_protocol_decoder_star_line_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderStarLine.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 */
void subghz_protocol_decoder_star_line_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_star_line_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_star_line_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderStarLine.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_star_line_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderStarLine.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_star_line_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderStarLine instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_star_line_get_string(void* context, FuriString* output);
