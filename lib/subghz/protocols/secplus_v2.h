#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_SECPLUS_V2_NAME "Security+ 2.0"

typedef struct SubGhzProtocolDecoderSecPlus_v2 SubGhzProtocolDecoderSecPlus_v2;
typedef struct SubGhzProtocolEncoderSecPlus_v2 SubGhzProtocolEncoderSecPlus_v2;

extern const SubGhzProtocolDecoder subghz_protocol_secplus_v2_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_secplus_v2_encoder;
extern const SubGhzProtocol subghz_protocol_secplus_v2;

/**
 * Allocate SubGhzProtocolEncoderSecPlus_v2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderSecPlus_v2* pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 */
void* subghz_protocol_encoder_secplus_v2_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 */
void subghz_protocol_encoder_secplus_v2_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 */
void subghz_protocol_encoder_secplus_v2_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_secplus_v2_yield(void* context);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 32 bit
 * @param btn Button number, 8 bit
 * @param cnt Container value, 28 bit
 * @param manufacture_name Name of manufacturer's key
 * @param preset Modulation, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_secplus_v2_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint32_t cnt,
    SubGhzPresetDefinition* preset);

/**
 * Allocate SubGhzProtocolDecoderSecPlus_v2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderSecPlus_v2* pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 */
void* subghz_protocol_decoder_secplus_v2_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 */
void subghz_protocol_decoder_secplus_v2_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 */
void subghz_protocol_decoder_secplus_v2_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_secplus_v2_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_secplus_v2_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_secplus_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_secplus_v2_get_string(void* context, string_t output);
