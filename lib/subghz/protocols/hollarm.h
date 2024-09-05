#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HOLLARM_NAME "Hollarm"

typedef struct SubGhzProtocolDecoderHollarm SubGhzProtocolDecoderHollarm;
typedef struct SubGhzProtocolEncoderHollarm SubGhzProtocolEncoderHollarm;

extern const SubGhzProtocolDecoder subghz_protocol_hollarm_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_hollarm_encoder;
extern const SubGhzProtocol subghz_protocol_hollarm;

/**
 * Allocate SubGhzProtocolEncoderHollarm.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHollarm* pointer to a SubGhzProtocolEncoderHollarm instance
 */
void* subghz_protocol_encoder_hollarm_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderHollarm.
 * @param context Pointer to a SubGhzProtocolEncoderHollarm instance
 */
void subghz_protocol_encoder_hollarm_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHollarm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_hollarm_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderHollarm instance
 */
void subghz_protocol_encoder_hollarm_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderHollarm instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_hollarm_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderHollarm.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHollarm* pointer to a SubGhzProtocolDecoderHollarm instance
 */
void* subghz_protocol_decoder_hollarm_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderHollarm.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 */
void subghz_protocol_decoder_hollarm_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderHollarm.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 */
void subghz_protocol_decoder_hollarm_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_hollarm_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_hollarm_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderHollarm.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_hollarm_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderHollarm.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_hollarm_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHollarm instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_hollarm_get_string(void* context, FuriString* output);
