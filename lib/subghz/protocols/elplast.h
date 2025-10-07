#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_ELPLAST_NAME "Elplast"

typedef struct SubGhzProtocolDecoderElplast SubGhzProtocolDecoderElplast;
typedef struct SubGhzProtocolEncoderElplast SubGhzProtocolEncoderElplast;

extern const SubGhzProtocolDecoder subghz_protocol_elplast_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_elplast_encoder;
extern const SubGhzProtocol subghz_protocol_elplast;

/**
 * Allocate SubGhzProtocolEncoderElplast.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderElplast* pointer to a SubGhzProtocolEncoderElplast instance
 */
void* subghz_protocol_encoder_elplast_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderElplast.
 * @param context Pointer to a SubGhzProtocolEncoderElplast instance
 */
void subghz_protocol_encoder_elplast_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderElplast instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_elplast_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderElplast instance
 */
void subghz_protocol_encoder_elplast_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderElplast instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_elplast_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderElplast.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderElplast* pointer to a SubGhzProtocolDecoderElplast instance
 */
void* subghz_protocol_decoder_elplast_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderElplast.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 */
void subghz_protocol_decoder_elplast_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderElplast.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 */
void subghz_protocol_decoder_elplast_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_elplast_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_elplast_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderElplast.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_elplast_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderElplast.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_elplast_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderElplast instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_elplast_get_string(void* context, FuriString* output);
