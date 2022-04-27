#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FAAC_SLH_NAME "Faac SLH"

typedef struct SubGhzProtocolDecoderFaacSLH SubGhzProtocolDecoderFaacSLH;
typedef struct SubGhzProtocolEncoderFaacSLH SubGhzProtocolEncoderFaacSLH;

extern const SubGhzProtocolDecoder subghz_protocol_faac_slh_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_faac_slh_encoder;
extern const SubGhzProtocol subghz_protocol_faac_slh;

/**
 * Allocate SubGhzProtocolEncoderFaacSLH.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderFaacSLH* pointer to a SubGhzProtocolEncoderFaacSLH instance
 */
void* subghz_protocol_encoder_faac_slh_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderFaacSLH.
 * @param context Pointer to a SubGhzProtocolEncoderFaacSLH instance
 */
void subghz_protocol_encoder_faac_slh_free(void* context);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderFaacSLH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 28 bit
 * @param btn Button number, 4 bit
 * @param cnt Counter value, 16 bit
 * @param seed Seed value, 32 bit
 * @param manufacture_name Name of manufacturer's key
 * @param frequency Transmission frequency, Hz
 * @param preset Modulation, FuriHalSubGhzPreset
 * @return true On success
 */
bool subghz_protocol_faac_slh_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    uint32_t seed,
    const char* manufacture_name,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderFaacSLH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderFaacSLH instance
 */
void subghz_protocol_encoder_faac_slh_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderFaacSLH instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_faac_slh_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderFaacSLH.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderFaacSLH* pointer to a SubGhzProtocolDecoderFaacSLH instance
 */
void* subghz_protocol_decoder_faac_slh_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderFaacSLH.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 */
void subghz_protocol_decoder_faac_slh_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderFaacSLH.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 */
void subghz_protocol_decoder_faac_slh_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_faac_slh_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_faac_slh_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderFaacSLH.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param frequency The frequency at which the signal was received, Hz
 * @param preset The modulation on which the signal was received, FuriHalSubGhzPreset
 * @return true On success
 */
bool subghz_protocol_decoder_faac_slh_serialize(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t frequency,
    FuriHalSubGhzPreset preset);

/**
 * Deserialize data SubGhzProtocolDecoderFaacSLH.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_faac_slh_get_string(void* context, string_t output);
