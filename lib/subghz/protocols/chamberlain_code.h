#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_CHAMB_CODE_NAME "Cham_Code"

typedef struct SubGhzProtocolDecoderChamb_Code SubGhzProtocolDecoderChamb_Code;
typedef struct SubGhzProtocolEncoderChamb_Code SubGhzProtocolEncoderChamb_Code;

extern const SubGhzProtocolDecoder subghz_protocol_chamb_code_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_chamb_code_encoder;
extern const SubGhzProtocol subghz_protocol_chamb_code;

/**
 * Allocate SubGhzProtocolEncoderChamb_Code.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderChamb_Code* pointer to a SubGhzProtocolEncoderChamb_Code instance
 */
void* subghz_protocol_encoder_chamb_code_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderChamb_Code.
 * @param context Pointer to a SubGhzProtocolEncoderChamb_Code instance
 */
void subghz_protocol_encoder_chamb_code_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderChamb_Code instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_chamb_code_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderChamb_Code instance
 */
void subghz_protocol_encoder_chamb_code_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderChamb_Code instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_chamb_code_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderChamb_Code.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderChamb_Code* pointer to a SubGhzProtocolDecoderChamb_Code instance
 */
void* subghz_protocol_decoder_chamb_code_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderChamb_Code.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 */
void subghz_protocol_decoder_chamb_code_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderChamb_Code.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 */
void subghz_protocol_decoder_chamb_code_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_chamb_code_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_chamb_code_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderChamb_Code.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_chamb_code_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderChamb_Code.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_chamb_code_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderChamb_Code instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_chamb_code_get_string(void* context, string_t output);
