#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MEGACODE_NAME "MegaCode"

typedef struct SubGhzProtocolDecoderMegaCode SubGhzProtocolDecoderMegaCode;
typedef struct SubGhzProtocolEncoderMegaCode SubGhzProtocolEncoderMegaCode;

extern const SubGhzProtocolDecoder subghz_protocol_megacode_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_megacode_encoder;
extern const SubGhzProtocol subghz_protocol_megacode;

/**
 * Allocate SubGhzProtocolEncoderMegaCode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMegaCode* pointer to a SubGhzProtocolEncoderMegaCode instance
 */
void* subghz_protocol_encoder_megacode_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderMegaCode.
 * @param context Pointer to a SubGhzProtocolEncoderMegaCode instance
 */
void subghz_protocol_encoder_megacode_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMegaCode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_megacode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderMegaCode instance
 */
void subghz_protocol_encoder_megacode_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderMegaCode instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_megacode_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderMegaCode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMegaCode* pointer to a SubGhzProtocolDecoderMegaCode instance
 */
void* subghz_protocol_decoder_megacode_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderMegaCode.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 */
void subghz_protocol_decoder_megacode_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderMegaCode.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 */
void subghz_protocol_decoder_megacode_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_megacode_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_megacode_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderMegaCode.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_megacode_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderMegaCode.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_megacode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMegaCode instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_megacode_get_string(void* context, FuriString* output);
