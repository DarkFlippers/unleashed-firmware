#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HOLTEK_HT12X_NAME "Holtek_HT12X"

typedef struct SubGhzProtocolDecoderHoltek_HT12X SubGhzProtocolDecoderHoltek_HT12X;
typedef struct SubGhzProtocolEncoderHoltek_HT12X SubGhzProtocolEncoderHoltek_HT12X;

extern const SubGhzProtocolDecoder subghz_protocol_holtek_th12x_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_holtek_th12x_encoder;
extern const SubGhzProtocol subghz_protocol_holtek_th12x;

/**
 * Allocate SubGhzProtocolEncoderHoltek_HT12X.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHoltek_HT12X* pointer to a SubGhzProtocolEncoderHoltek_HT12X instance
 */
void* subghz_protocol_encoder_holtek_th12x_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderHoltek_HT12X.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek_HT12X instance
 */
void subghz_protocol_encoder_holtek_th12x_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek_HT12X instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_holtek_th12x_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek_HT12X instance
 */
void subghz_protocol_encoder_holtek_th12x_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderHoltek_HT12X instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_holtek_th12x_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderHoltek_HT12X.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHoltek_HT12X* pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 */
void* subghz_protocol_decoder_holtek_th12x_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderHoltek_HT12X.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 */
void subghz_protocol_decoder_holtek_th12x_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderHoltek_HT12X.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 */
void subghz_protocol_decoder_holtek_th12x_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_holtek_th12x_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_holtek_th12x_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderHoltek_HT12X.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_holtek_th12x_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderHoltek_HT12X.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_holtek_th12x_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHoltek_HT12X instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_holtek_th12x_get_string(void* context, FuriString* output);
