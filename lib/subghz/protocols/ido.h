#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_IDO_NAME "iDo 117/111"

typedef struct SubGhzProtocolDecoderIDo SubGhzProtocolDecoderIDo;
typedef struct SubGhzProtocolEncoderIDo SubGhzProtocolEncoderIDo;

extern const SubGhzProtocolDecoder subghz_protocol_ido_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_ido_encoder;
extern const SubGhzProtocol subghz_protocol_ido;

/**
 * Allocate SubGhzProtocolDecoderIDo.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderIDo* pointer to a SubGhzProtocolDecoderIDo instance
 */
void* subghz_protocol_decoder_ido_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderIDo instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_ido_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderIDo.
 * @param context Pointer to a SubGhzProtocolDecoderIDo instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_ido_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderIDo instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_ido_get_string(void* context, FuriString* output);
