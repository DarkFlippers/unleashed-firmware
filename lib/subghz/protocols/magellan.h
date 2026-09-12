#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MAGELLAN_NAME "Magellan"

typedef struct SubGhzProtocolDecoderMagellan SubGhzProtocolDecoderMagellan;
typedef struct SubGhzProtocolEncoderMagellan SubGhzProtocolEncoderMagellan;

extern const SubGhzProtocolDecoder subghz_protocol_magellan_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_magellan_encoder;
extern const SubGhzProtocol subghz_protocol_magellan;

/**
 * Allocate SubGhzProtocolEncoderMagellan.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMagellan* pointer to a SubGhzProtocolEncoderMagellan instance
 */
void* subghz_protocol_encoder_magellan_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMagellan instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_magellan_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderMagellan instance
 */
void subghz_protocol_encoder_magellan_stop(void* context);

/**
 * Allocate SubGhzProtocolDecoderMagellan.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMagellan* pointer to a SubGhzProtocolDecoderMagellan instance
 */
void* subghz_protocol_decoder_magellan_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMagellan instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_magellan_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderMagellan.
 * @param context Pointer to a SubGhzProtocolDecoderMagellan instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_magellan_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMagellan instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_magellan_get_string(void* context, FuriString* output);
