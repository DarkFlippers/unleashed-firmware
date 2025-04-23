#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_REVERSRB2_NAME "Revers_RB2"

typedef struct SubGhzProtocolDecoderRevers_RB2 SubGhzProtocolDecoderRevers_RB2;
typedef struct SubGhzProtocolEncoderRevers_RB2 SubGhzProtocolEncoderRevers_RB2;

extern const SubGhzProtocolDecoder subghz_protocol_revers_rb2_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_revers_rb2_encoder;
extern const SubGhzProtocol subghz_protocol_revers_rb2;

/**
 * Allocate SubGhzProtocolEncoderRevers_RB2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderRevers_RB2* pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 */
void* subghz_protocol_encoder_revers_rb2_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 */
void subghz_protocol_encoder_revers_rb2_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_revers_rb2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 */
void subghz_protocol_encoder_revers_rb2_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderRevers_RB2 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_revers_rb2_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderRevers_RB2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderRevers_RB2* pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 */
void* subghz_protocol_decoder_revers_rb2_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 */
void subghz_protocol_decoder_revers_rb2_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 */
void subghz_protocol_decoder_revers_rb2_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_revers_rb2_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_revers_rb2_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_revers_rb2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderRevers_RB2.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_revers_rb2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderRevers_RB2 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_revers_rb2_get_string(void* context, FuriString* output);
