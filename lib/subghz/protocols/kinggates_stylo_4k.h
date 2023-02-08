#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_KINGGATES_STYLO_4K_NAME "KingGates Stylo4k"

typedef struct SubGhzProtocolDecoderKingGates_stylo_4k SubGhzProtocolDecoderKingGates_stylo_4k;
typedef struct SubGhzProtocolEncoderKingGates_stylo_4k SubGhzProtocolEncoderKingGates_stylo_4k;

extern const SubGhzProtocolDecoder subghz_protocol_kinggates_stylo_4k_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_kinggates_stylo_4k_encoder;
extern const SubGhzProtocol subghz_protocol_kinggates_stylo_4k;

/**
 * Allocate SubGhzProtocolDecoderKingGates_stylo_4k.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderKingGates_stylo_4k* pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 */
void* subghz_protocol_decoder_kinggates_stylo_4k_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderKingGates_stylo_4k.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 */
void subghz_protocol_decoder_kinggates_stylo_4k_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderKingGates_stylo_4k.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 */
void subghz_protocol_decoder_kinggates_stylo_4k_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_kinggates_stylo_4k_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_kinggates_stylo_4k_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderKingGates_stylo_4k.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_decoder_kinggates_stylo_4k_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderKingGates_stylo_4k.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_kinggates_stylo_4k_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderKingGates_stylo_4k instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_kinggates_stylo_4k_get_string(void* context, FuriString* output);
