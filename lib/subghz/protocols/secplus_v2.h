#pragma once

#include "base.h"
#include "public_api.h"

#define SUBGHZ_PROTOCOL_SECPLUS_V2_NAME "Security+ 2.0"

typedef struct SubGhzProtocolDecoderSecPlus_v2 SubGhzProtocolDecoderSecPlus_v2;
typedef struct SubGhzProtocolEncoderSecPlus_v2 SubGhzProtocolEncoderSecPlus_v2;

extern const SubGhzProtocolDecoder subghz_protocol_secplus_v2_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_secplus_v2_encoder;
extern const SubGhzProtocol subghz_protocol_secplus_v2;

/**
 * Allocate SubGhzProtocolEncoderSecPlus_v2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderSecPlus_v2* pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 */
void* subghz_protocol_encoder_secplus_v2_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderSecPlus_v2 instance
 */
void subghz_protocol_encoder_secplus_v2_stop(void* context);

/**
 * Allocate SubGhzProtocolDecoderSecPlus_v2.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderSecPlus_v2* pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 */
void* subghz_protocol_decoder_secplus_v2_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 */
void subghz_protocol_decoder_secplus_v2_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_secplus_v2_feed(void* context, bool level, uint32_t duration);

/**
 * Serialize data SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_secplus_v2_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderSecPlus_v2.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_secplus_v2_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderSecPlus_v2 instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_secplus_v2_get_string(void* context, FuriString* output);
