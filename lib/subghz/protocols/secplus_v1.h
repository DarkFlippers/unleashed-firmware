#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_SECPLUS_V1_NAME "Security+ 1.0"

typedef struct SubGhzProtocolDecoderSecPlus_v1 SubGhzProtocolDecoderSecPlus_v1;
typedef struct SubGhzProtocolEncoderSecPlus_v1 SubGhzProtocolEncoderSecPlus_v1;

extern const SubGhzProtocolDecoder subghz_protocol_secplus_v1_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_secplus_v1_encoder;
extern const SubGhzProtocol subghz_protocol_secplus_v1;

/**
 * Allocate SubGhzProtocolEncoderSecPlus_v1.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderSecPlus_v1* pointer to a SubGhzProtocolEncoderSecPlus_v1 instance
 */
void* subghz_protocol_encoder_secplus_v1_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderSecPlus_v1.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v1 instance
 */
void subghz_protocol_encoder_secplus_v1_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v1 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_secplus_v1_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v1 instance
 */
void subghz_protocol_encoder_secplus_v1_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v1 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_secplus_v1_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderSecPlus_v1.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderSecPlus_v1* pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 */
void* subghz_protocol_decoder_secplus_v1_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderSecPlus_v1.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 */
void subghz_protocol_decoder_secplus_v1_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderSecPlus_v1.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 */
void subghz_protocol_decoder_secplus_v1_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_secplus_v1_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_secplus_v1_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderSecPlus_v1.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_secplus_v1_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderSecPlus_v1.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_secplus_v1_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Validation of fixed parts SubGhzProtocolDecoderSecPlus_v1.
 * @param fixed fixed parts
 * @return true On success
 */
bool subghz_protocol_secplus_v1_check_fixed(uint32_t fixed);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v1 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_secplus_v1_get_string(void* context, FuriString* output);
