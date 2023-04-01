#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_SOMFY_TELIS_NAME "Somfy Telis"

typedef struct SubGhzProtocolDecoderSomfyTelis SubGhzProtocolDecoderSomfyTelis;
typedef struct SubGhzProtocolEncoderSomfyTelis SubGhzProtocolEncoderSomfyTelis;

extern const SubGhzProtocolDecoder subghz_protocol_somfy_telis_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_somfy_telis_encoder;
extern const SubGhzProtocol subghz_protocol_somfy_telis;

/**
 * Allocate SubGhzProtocolEncoderSomfyTelis.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderSomfyTelis* pointer to a SubGhzProtocolEncoderSomfyTelis instance
 */
void* subghz_protocol_encoder_somfy_telis_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderSomfyTelis.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 */
void subghz_protocol_encoder_somfy_telis_free(void* context);

/**
 * Key generation from simple data.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param serial Serial number, 24 bit
 * @param btn Button number, 8 bit
 * @param cnt Counter value, 16 bit
 * @param preset Modulation, SubGhzRadioPreset
 * @return true On success
 */
bool subghz_protocol_somfy_telis_create_data(
    void* context,
    FlipperFormat* flipper_format,
    uint32_t serial,
    uint8_t btn,
    uint16_t cnt,
    SubGhzRadioPreset* preset);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_somfy_telis_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 */
void subghz_protocol_encoder_somfy_telis_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderSomfyTelis instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_somfy_telis_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderSomfyTelis.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderSomfyTelis* pointer to a SubGhzProtocolDecoderSomfyTelis instance
 */
void* subghz_protocol_decoder_somfy_telis_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderSomfyTelis.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 */
void subghz_protocol_decoder_somfy_telis_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderSomfyTelis.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 */
void subghz_protocol_decoder_somfy_telis_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_somfy_telis_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_somfy_telis_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderSomfyTelis.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_somfy_telis_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderSomfyTelis.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_somfy_telis_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderSomfyTelis instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_somfy_telis_get_string(void* context, FuriString* output);
