#pragma once
#include "base.h"

#define SUBGHZ_PROTOCOL_CAME_ATOMO_NAME "CAME Atomo"

typedef struct SubGhzProtocolDecoderCameAtomo SubGhzProtocolDecoderCameAtomo;
typedef struct SubGhzProtocolEncoderCameAtomo SubGhzProtocolEncoderCameAtomo;

extern const SubGhzProtocolDecoder subghz_protocol_came_atomo_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_came_atomo_encoder;
extern const SubGhzProtocol subghz_protocol_came_atomo;

void atomo_decrypt(uint8_t* buff);

void atomo_encrypt(uint8_t* buff);

/**
 * Allocate SubGhzProtocolEncoderCameAtomo.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderCameAtomo* pointer to a SubGhzProtocolEncoderCameAtomo instance
 */
void* subghz_protocol_encoder_came_atomo_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderCameAtomo instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_came_atomo_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderCameAtomo.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderCameAtomo* pointer to a SubGhzProtocolDecoderCameAtomo instance
 */
void* subghz_protocol_decoder_came_atomo_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderCameAtomo.
 * @param context Pointer to a SubGhzProtocolDecoderCameAtomo instance
 */
void subghz_protocol_decoder_came_atomo_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderCameAtomo instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_came_atomo_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderCameAtomo.
 * @param context Pointer to a SubGhzProtocolDecoderCameAtomo instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_came_atomo_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderCameAtomo instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_came_atomo_get_string(void* context, FuriString* output);
