#pragma once

#include "base.h"
#include "public_api.h"

#define SUBGHZ_PROTOCOL_KEELOQ_NAME "KeeLoq"

typedef struct SubGhzProtocolDecoderKeeloq SubGhzProtocolDecoderKeeloq;
typedef struct SubGhzProtocolEncoderKeeloq SubGhzProtocolEncoderKeeloq;

extern const SubGhzProtocolDecoder subghz_protocol_keeloq_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_keeloq_encoder;
extern const SubGhzProtocol subghz_protocol_keeloq;

/**
 * Allocate SubGhzProtocolEncoderKeeloq.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderKeeloq* pointer to a SubGhzProtocolEncoderKeeloq instance
 */
void* subghz_protocol_encoder_keeloq_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderKeeloq.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 */
void subghz_protocol_encoder_keeloq_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_keeloq_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 */
void subghz_protocol_encoder_keeloq_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderKeeloq instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_keeloq_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderKeeloq.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderKeeloq* pointer to a SubGhzProtocolDecoderKeeloq instance
 */
void* subghz_protocol_decoder_keeloq_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderKeeloq.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 */
void subghz_protocol_decoder_keeloq_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderKeeloq.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 */
void subghz_protocol_decoder_keeloq_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_keeloq_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_keeloq_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderKeeloq.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return SubGhzProtocolStatus
 */
SubGhzProtocolStatus subghz_protocol_decoder_keeloq_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderKeeloq.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return SubGhzProtocolStatus
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_keeloq_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderKeeloq instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_keeloq_get_string(void* context, FuriString* output);
