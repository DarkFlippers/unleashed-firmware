#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_ALUTECH_AT_4N_NAME "Alutech AT-4N"

typedef struct SubGhzProtocolDecoderAlutech_at_4n SubGhzProtocolDecoderAlutech_at_4n;
typedef struct SubGhzProtocolEncoderAlutech_at_4n SubGhzProtocolEncoderAlutech_at_4n;

extern const SubGhzProtocolDecoder subghz_protocol_alutech_at_4n_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_alutech_at_4n_encoder;
extern const SubGhzProtocol subghz_protocol_alutech_at_4n;

/**
 * Allocate SubGhzProtocolEncoderAlutech_at_4n.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderAlutech_at_4n* pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 */
void* subghz_protocol_encoder_alutech_at_4n_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderAlutech_at_4n.
 * @param context Pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 */
void subghz_protocol_encoder_alutech_at_4n_free(void* context);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_alutech_at_4n_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_alutech_at_4n_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 */
void subghz_protocol_encoder_alutech_at_4n_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderAlutech_at_4n instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_alutech_at_4n_yield(void* context);

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
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_alutech_at_4n_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderAlutech_at_4n.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_alutech_at_4n_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderAlutech_at_4n instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_alutech_at_4n_get_string(void* context, FuriString* output);
