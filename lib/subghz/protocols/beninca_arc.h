#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_BENINCA_ARC_NAME "Beninca ARC"

typedef struct SubGhzProtocolDecoderBenincaARC SubGhzProtocolDecoderBenincaARC;
typedef struct SubGhzProtocolEncoderBenincaARC SubGhzProtocolEncoderBenincaARC;

extern const SubGhzProtocolDecoder subghz_protocol_beninca_arc_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_beninca_arc_encoder;
extern const SubGhzProtocol subghz_protocol_beninca_arc;

/**
 * Allocate SubGhzProtocolEncoderBenincaARC.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderBenincaARC* pointer to a SubGhzProtocolEncoderBenincaARC instance
 */
void* subghz_protocol_encoder_beninca_arc_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderBenincaARC.
 * @param context Pointer to a SubGhzProtocolEncoderBenincaARC instance
 */
void subghz_protocol_encoder_beninca_arc_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderBenincaARC instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_beninca_arc_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderBenincaARC instance
 */
void subghz_protocol_encoder_beninca_arc_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderBenincaARC instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_beninca_arc_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderBenincaARC.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderBenincaARC* pointer to a SubGhzProtocolDecoderBenincaARC instance
 */
void* subghz_protocol_decoder_beninca_arc_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderBenincaARC.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 */
void subghz_protocol_decoder_beninca_arc_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderBenincaARC.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 */
void subghz_protocol_decoder_beninca_arc_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_beninca_arc_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_beninca_arc_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderBenincaARC.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_beninca_arc_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderBenincaARC.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_beninca_arc_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderBenincaARC instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_beninca_arc_get_string(void* context, FuriString* output);
