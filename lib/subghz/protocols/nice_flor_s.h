#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_NICE_FLOR_S_NAME "Nice FloR-S"

typedef struct SubGhzProtocolDecoderNiceFlorS SubGhzProtocolDecoderNiceFlorS;
typedef struct SubGhzProtocolEncoderNiceFlorS SubGhzProtocolEncoderNiceFlorS;

extern const SubGhzProtocolDecoder subghz_protocol_nice_flor_s_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_nice_flor_s_encoder;
extern const SubGhzProtocol subghz_protocol_nice_flor_s;

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
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_nice_flor_s_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderNiceFlorS.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_nice_flor_s_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderNiceFlorS instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_nice_flor_s_get_string(void* context, string_t output);
