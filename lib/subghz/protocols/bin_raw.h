#pragma once

#include "base.h"
#include "public_api.h"

#define SUBGHZ_PROTOCOL_BIN_RAW_NAME "BinRAW"

typedef struct SubGhzProtocolDecoderBinRAW SubGhzProtocolDecoderBinRAW;
typedef struct SubGhzProtocolEncoderBinRAW SubGhzProtocolEncoderBinRAW;

extern const SubGhzProtocolDecoder subghz_protocol_bin_raw_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_bin_raw_encoder;
extern const SubGhzProtocol subghz_protocol_bin_raw;

/**
 * Allocate SubGhzProtocolEncoderBinRAW.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderBinRAW* pointer to a SubGhzProtocolEncoderBinRAW instance
 */
void* subghz_protocol_encoder_bin_raw_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderBinRAW.
 * @param context Pointer to a SubGhzProtocolEncoderBinRAW instance
 */
void subghz_protocol_encoder_bin_raw_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderBinRAW instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_bin_raw_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderBinRAW instance
 */
void subghz_protocol_encoder_bin_raw_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderBinRAW instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_bin_raw_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderBinRAW.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderBinRAW* pointer to a SubGhzProtocolDecoderBinRAW instance
 */
void* subghz_protocol_decoder_bin_raw_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderBinRAW.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 */
void subghz_protocol_decoder_bin_raw_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderBinRAW.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 */
void subghz_protocol_decoder_bin_raw_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_bin_raw_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_bin_raw_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderBinRAW.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_bin_raw_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderBinRAW.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_bin_raw_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderBinRAW instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_bin_raw_get_string(void* context, FuriString* output);
