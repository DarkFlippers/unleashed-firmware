#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_BETT_NAME "BETT"

typedef struct SubGhzProtocolDecoderBETT SubGhzProtocolDecoderBETT;
typedef struct SubGhzProtocolEncoderBETT SubGhzProtocolEncoderBETT;

extern const SubGhzProtocolDecoder subghz_protocol_bett_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_bett_encoder;
extern const SubGhzProtocol subghz_protocol_bett;

/**
 * Allocate SubGhzProtocolEncoderBETT.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderBETT* pointer to a SubGhzProtocolEncoderBETT instance
 */
void* subghz_protocol_encoder_bett_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderBETT.
 * @param context Pointer to a SubGhzProtocolEncoderBETT instance
 */
void subghz_protocol_encoder_bett_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderBETT instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_bett_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderBETT instance
 */
void subghz_protocol_encoder_bett_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderBETT instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_bett_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderBETT.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderBETT* pointer to a SubGhzProtocolDecoderBETT instance
 */
void* subghz_protocol_decoder_bett_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderBETT.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 */
void subghz_protocol_decoder_bett_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderBETT.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 */
void subghz_protocol_decoder_bett_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_bett_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_bett_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderBETT.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_bett_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderBETT.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_bett_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderBETT instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_bett_get_string(void* context, string_t output);
