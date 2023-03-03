#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_FAAC_SLH_NAME "Faac SLH"

typedef struct SubGhzProtocolDecoderFaacSLH SubGhzProtocolDecoderFaacSLH;
typedef struct SubGhzProtocolEncoderFaacSLH SubGhzProtocolEncoderFaacSLH;

extern const SubGhzProtocolDecoder subghz_protocol_faac_slh_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_faac_slh_encoder;
extern const SubGhzProtocol subghz_protocol_faac_slh;

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
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_faac_slh_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderFaacSLH.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_faac_slh_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderFaacSLH instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_faac_slh_get_string(void* context, FuriString* output);
