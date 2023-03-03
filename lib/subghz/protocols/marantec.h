#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MARANTEC_NAME "Marantec"

typedef struct SubGhzProtocolDecoderMarantec SubGhzProtocolDecoderMarantec;
typedef struct SubGhzProtocolEncoderMarantec SubGhzProtocolEncoderMarantec;

extern const SubGhzProtocolDecoder subghz_protocol_marantec_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_marantec_encoder;
extern const SubGhzProtocol subghz_protocol_marantec;

/**
 * Allocate SubGhzProtocolEncoderMarantec.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMarantec* pointer to a SubGhzProtocolEncoderMarantec instance
 */
void* subghz_protocol_encoder_marantec_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderMarantec.
 * @param context Pointer to a SubGhzProtocolEncoderMarantec instance
 */
void subghz_protocol_encoder_marantec_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMarantec instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_marantec_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderMarantec instance
 */
void subghz_protocol_encoder_marantec_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderMarantec instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_marantec_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderMarantec.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMarantec* pointer to a SubGhzProtocolDecoderMarantec instance
 */
void* subghz_protocol_decoder_marantec_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderMarantec.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 */
void subghz_protocol_decoder_marantec_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderMarantec.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 */
void subghz_protocol_decoder_marantec_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_marantec_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_marantec_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderMarantec.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_marantec_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderMarantec.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_marantec_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_marantec_get_string(void* context, FuriString* output);
