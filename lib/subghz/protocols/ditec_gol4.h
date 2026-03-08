#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_DITEC_GOL4_NAME "Ditec GOL4"

typedef struct SubGhzProtocolDecoderDitecGOL4 SubGhzProtocolDecoderDitecGOL4;
typedef struct SubGhzProtocolEncoderDitecGOL4 SubGhzProtocolEncoderDitecGOL4;

extern const SubGhzProtocolDecoder subghz_protocol_ditec_gol4_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_ditec_gol4_encoder;
extern const SubGhzProtocol subghz_protocol_ditec_gol4;

/**
 * Allocate SubGhzProtocolEncoderDitecGOL4.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderDitecGOL4* pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 */
void* subghz_protocol_encoder_ditec_gol4_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderDitecGOL4.
 * @param context Pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 */
void subghz_protocol_encoder_ditec_gol4_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_ditec_gol4_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 */
void subghz_protocol_encoder_ditec_gol4_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderDitecGOL4 instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_ditec_gol4_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderDitecGOL4.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderDitecGOL4* pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 */
void* subghz_protocol_decoder_ditec_gol4_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderDitecGOL4.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 */
void subghz_protocol_decoder_ditec_gol4_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderDitecGOL4.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 */
void subghz_protocol_decoder_ditec_gol4_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_ditec_gol4_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_ditec_gol4_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderDitecGOL4.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_ditec_gol4_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderDitecGOL4.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_ditec_gol4_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderDitecGOL4 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_ditec_gol4_get_string(void* context, FuriString* output);
