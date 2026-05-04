#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NORD_ICE_NAME "Nord ICE"

typedef struct SubGhzProtocolDecoderNord_Ice SubGhzProtocolDecoderNord_Ice;
typedef struct SubGhzProtocolEncoderNord_Ice SubGhzProtocolEncoderNord_Ice;

extern const SubGhzProtocolDecoder subghz_protocol_nord_ice_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nord_ice_encoder;
extern const SubGhzProtocol subghz_protocol_nord_ice;

/**
 * Allocate SubGhzProtocolEncoderNord_Ice.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderNord_Ice* pointer to a SubGhzProtocolEncoderNord_Ice instance
 */
void* subghz_protocol_encoder_nord_ice_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderNord_Ice.
 * @param context Pointer to a SubGhzProtocolEncoderNord_Ice instance
 */
void subghz_protocol_encoder_nord_ice_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderNord_Ice instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_nord_ice_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderNord_Ice instance
 */
void subghz_protocol_encoder_nord_ice_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderNord_Ice instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_nord_ice_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderNord_Ice.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderNord_Ice* pointer to a SubGhzProtocolDecoderNord_Ice instance
 */
void* subghz_protocol_decoder_nord_ice_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderNord_Ice.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 */
void subghz_protocol_decoder_nord_ice_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderNord_Ice.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 */
void subghz_protocol_decoder_nord_ice_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_nord_ice_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_nord_ice_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderNord_Ice.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_nord_ice_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderNord_Ice.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_nord_ice_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNord_Ice instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nord_ice_get_string(void* context, FuriString* output);
