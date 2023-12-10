#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MASTERCODE_NAME "Mastercode"

typedef struct SubGhzProtocolDecoderMastercode SubGhzProtocolDecoderMastercode;
typedef struct SubGhzProtocolEncoderMastercode SubGhzProtocolEncoderMastercode;

extern const SubGhzProtocolDecoder subghz_protocol_mastercode_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_mastercode_encoder;
extern const SubGhzProtocol subghz_protocol_mastercode;

/**
 * Allocate SubGhzProtocolEncoderMastercode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMastercode* pointer to a SubGhzProtocolEncoderMastercode instance
 */
void* subghz_protocol_encoder_mastercode_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderMastercode.
 * @param context Pointer to a SubGhzProtocolEncoderMastercode instance
 */
void subghz_protocol_encoder_mastercode_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMastercode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_mastercode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderMastercode instance
 */
void subghz_protocol_encoder_mastercode_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderMastercode instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_mastercode_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderMastercode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMastercode* pointer to a SubGhzProtocolDecoderMastercode instance
 */
void* subghz_protocol_decoder_mastercode_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderMastercode.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 */
void subghz_protocol_decoder_mastercode_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderMastercode.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 */
void subghz_protocol_decoder_mastercode_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_mastercode_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_mastercode_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderMastercode.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_mastercode_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderMastercode.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_mastercode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_mastercode_get_string(void* context, FuriString* output);
