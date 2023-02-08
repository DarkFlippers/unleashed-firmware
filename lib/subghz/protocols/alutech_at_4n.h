#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME "Alutech at-4n"

typedef struct SubGhzProtocolDecoderAlutech_at_4n SubGhzProtocolDecoderAlutech_at_4n;
typedef struct SubGhzProtocolEncoderAlutech_at_4n SubGhzProtocolEncoderAlutech_at_4n;

extern const SubGhzProtocolDecoder subghz_protocol_alutech_at_4n_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_alutech_at_4n_encoder;
extern const SubGhzProtocol subghz_protocol_alutech_at_4n;

/**
 * Allocate SubGhzProtocolDecoderAlutech_at_4n.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderAlutech_at_4n* pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 */
void* subghz_protocol_decoder_alutech_at_4n_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderAlutech_at_4n.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 */
void subghz_protocol_decoder_alutech_at_4n_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderAlutech_at_4n.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 */
void subghz_protocol_decoder_alutech_at_4n_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_alutech_at_4n_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_alutech_at_4n_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderAlutech_at_4n.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_decoder_alutech_at_4n_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderAlutech_at_4n.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_alutech_at_4n_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_alutech_at_4n_get_string(void* context, FuriString* output);
