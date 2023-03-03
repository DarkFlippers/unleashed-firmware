#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NERO_SKETCH_NAME "Nero Sketch"

typedef struct SubGhzProtocolDecoderNeroSketch SubGhzProtocolDecoderNeroSketch;
typedef struct SubGhzProtocolEncoderNeroSketch SubGhzProtocolEncoderNeroSketch;

extern const SubGhzProtocolDecoder subghz_protocol_nero_sketch_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nero_sketch_encoder;
extern const SubGhzProtocol subghz_protocol_nero_sketch;

/**
 * Allocate SubGhzProtocolEncoderNeroSketch.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderNeroSketch* pointer to a SubGhzProtocolEncoderNeroSketch instance
 */
void* subghz_protocol_encoder_nero_sketch_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderNeroSketch.
 * @param context Pointer to a SubGhzProtocolEncoderNeroSketch instance
 */
void subghz_protocol_encoder_nero_sketch_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderNeroSketch instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_nero_sketch_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderNeroSketch instance
 */
void subghz_protocol_encoder_nero_sketch_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderNeroSketch instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_nero_sketch_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderNeroSketch.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderNeroSketch* pointer to a SubGhzProtocolDecoderNeroSketch instance
 */
void* subghz_protocol_decoder_nero_sketch_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderNeroSketch.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 */
void subghz_protocol_decoder_nero_sketch_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderNeroSketch.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 */
void subghz_protocol_decoder_nero_sketch_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_nero_sketch_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_nero_sketch_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderNeroSketch.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_nero_sketch_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderNeroSketch.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_nero_sketch_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNeroSketch instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nero_sketch_get_string(void* context, FuriString* output);
