#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_STAR_LINE_NAME "Star Line"

typedef struct SubGhzProtocolDecoderStarLine SubGhzProtocolDecoderStarLine;
typedef struct SubGhzProtocolEncoderStarLine SubGhzProtocolEncoderStarLine;

extern const SubGhzProtocolDecoder subghz_protocol_star_line_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_star_line_encoder;
extern const SubGhzProtocol subghz_protocol_star_line;

/**
 * Allocate SubGhzProtocolEncoderStarLine.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderStarLine* pointer to a SubGhzProtocolEncoderStarLine instance
 */
void* subghz_protocol_encoder_star_line_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderStarLine.
 * @param context Pointer to a SubGhzProtocolEncoderStarLine instance
 */
void subghz_protocol_encoder_star_line_free(void* context);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderStarLine instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_star_line_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    const char* manufacture_name,
    SubGhzRadioPreset* preset);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderStarLine instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_star_line_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderStarLine instance
 */
void subghz_protocol_encoder_star_line_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderStarLine instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_star_line_yield(void* context);

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
