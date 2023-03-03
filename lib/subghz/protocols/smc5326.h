#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_SMC5326_NAME "SMC5326"

typedef struct SubGhzProtocolDecoderSMC5326 SubGhzProtocolDecoderSMC5326;
typedef struct SubGhzProtocolEncoderSMC5326 SubGhzProtocolEncoderSMC5326;

extern const SubGhzProtocolDecoder subghz_protocol_smc5326_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_smc5326_encoder;
extern const SubGhzProtocol subghz_protocol_smc5326;

/**
 * Allocate SubGhzProtocolEncoderSMC5326.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderSMC5326* pointer to a SubGhzProtocolEncoderSMC5326 instance
 */
void* subghz_protocol_encoder_smc5326_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderSMC5326.
 * @param context Pointer to a SubGhzProtocolEncoderSMC5326 instance
 */
void subghz_protocol_encoder_smc5326_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderSMC5326 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_smc5326_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderSMC5326 instance
 */
void subghz_protocol_encoder_smc5326_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderSMC5326 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_smc5326_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderSMC5326.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderSMC5326* pointer to a SubGhzProtocolDecoderSMC5326 instance
 */
void* subghz_protocol_decoder_smc5326_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderSMC5326.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 */
void subghz_protocol_decoder_smc5326_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderSMC5326.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 */
void subghz_protocol_decoder_smc5326_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_smc5326_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_smc5326_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderSMC5326.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_smc5326_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderSMC5326.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_smc5326_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderSMC5326 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_smc5326_get_string(void* context, FuriString* output);
