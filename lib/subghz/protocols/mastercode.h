#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_MASTERCODE_NAME "Mastercode"

typedef struct SubGhzProtocolDecoderMastercode SubGhzProtocolDecoderMastercode;
typedef struct SubGhzProtocolEncoderMastercode SubGhzProtocolEncoderMastercode;

extern const SubGhzProtocolDecoder subghz_protocol_mastercode_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_mastercode_encoder;
extern const SubGhzProtocol subghz_protocol_mastercode;

/**
 * Allocate SubGhzProtocolEncoderMastercode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderMastercode* pointer to a SubGhzProtocolEncoderMastercode instance
 */
void* subghz_protocol_encoder_mastercode_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderMastercode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_mastercode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Allocate SubGhzProtocolDecoderMastercode.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderMastercode* pointer to a SubGhzProtocolDecoderMastercode instance
 */
void* subghz_protocol_decoder_mastercode_alloc(SubGhzEnvironment* environment);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_mastercode_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderMastercode.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_mastercode_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderMastercode instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_mastercode_get_string(void* context, FuriString* output);
