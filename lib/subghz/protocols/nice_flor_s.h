#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME "Nice FloR-S"

typedef struct SubGhzProtocolDecoderNiceFlorS SubGhzProtocolDecoderNiceFlorS;
typedef struct SubGhzProtocolEncoderNiceFlorS SubGhzProtocolEncoderNiceFlorS;

extern const SubGhzProtocolDecoder subghz_protocol_nice_flor_s_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nice_flor_s_encoder;
extern const SubGhzProtocol subghz_protocol_nice_flor_s;

/**
 * Allocate SubGhzProtocolEncoderNiceFlorS.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderNiceFlorS* pointer to a SubGhzProtocolEncoderNiceFlorS instance
 */
void* subghz_protocol_encoder_nice_flor_s_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderNiceFlorS.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 */
void subghz_protocol_encoder_nice_flor_s_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_nice_flor_s_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 */
void subghz_protocol_encoder_nice_flor_s_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_nice_flor_s_yield(void* context);

uint64_t subghz_protocol_nice_flor_s_encrypt(uint64_t data, const char* file_name);

/**
 * New remote generation.
 * @param context Pointer to a SubGhzProtocolEncoderNiceFlorS instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number
 * @param btn Button number, 4 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @param nice_one Nice One if true, Nice Flor S if false
 * @return true On success
 */
bool subghz_protocol_nice_flor_s_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset,
    bool nice_one);

/**
 * Allocate SubGhzProtocolDecoderNiceFlorS.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderNiceFlorS* pointer to a SubGhzProtocolDecoderNiceFlorS instance
 */
void* subghz_protocol_decoder_nice_flor_s_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderNiceFlorS.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 */
void subghz_protocol_decoder_nice_flor_s_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderNiceFlorS.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 */
void subghz_protocol_decoder_nice_flor_s_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_nice_flor_s_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_nice_flor_s_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderNiceFlorS.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_nice_flor_s_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderNiceFlorS.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_nice_flor_s_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nice_flor_s_get_string(void* context, FuriString* output);
