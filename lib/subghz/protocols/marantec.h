#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MARANTEC_NAME "Marantec"

typedef struct SubGhzProtocolDecoderMarantec SubGhzProtocolDecoderMarantec;
typedef struct SubGhzProtocolEncoderMarantec SubGhzProtocolEncoderMarantec;

extern const SubGhzProtocolDecoder subghz_protocol_marantec_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_marantec_encoder;
extern const SubGhzProtocol subghz_protocol_marantec;

/**
 * Allocate SubGhzProtocolEncoderMarantec.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMarantec* pointer to a SubGhzProtocolEncoderMarantec instance
 */
void* subghz_protocol_encoder_marantec_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMarantec instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_marantec_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderMarantec instance
 */
void subghz_protocol_encoder_marantec_stop(void* context);

/**
 * Allocate SubGhzProtocolDecoderMarantec.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMarantec* pointer to a SubGhzProtocolDecoderMarantec instance
 */
void* subghz_protocol_decoder_marantec_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderMarantec.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 */
void subghz_protocol_decoder_marantec_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_marantec_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderMarantec.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_marantec_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMarantec instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_marantec_get_string(void* context, FuriString* output);

/**
 * Calculate CRC8 for Marantec protocol.
 * @param data Pointer to the data buffer
 * @param len Length of the data buffer
 * @return CRC8 value
 */
uint8_t subghz_protocol_marantec_crc8(uint8_t* data, size_t len);
