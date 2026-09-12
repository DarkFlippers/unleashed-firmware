#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_POWER_SMART_NAME "Power Smart"

typedef struct SubGhzProtocolDecoderPowerSmart SubGhzProtocolDecoderPowerSmart;
typedef struct SubGhzProtocolEncoderPowerSmart SubGhzProtocolEncoderPowerSmart;

extern const SubGhzProtocolDecoder subghz_protocol_power_smart_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_power_smart_encoder;
extern const SubGhzProtocol subghz_protocol_power_smart;

/**
 * Allocate SubGhzProtocolEncoderPowerSmart.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderPowerSmart* pointer to a SubGhzProtocolEncoderPowerSmart instance
 */
void* subghz_protocol_encoder_power_smart_alloc(SubGhzEnvironment* environment);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderPowerSmart instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_power_smart_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderPowerSmart instance
 */
void subghz_protocol_encoder_power_smart_stop(void* context);

/**
 * Allocate SubGhzProtocolDecoderPowerSmart.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderPowerSmart* pointer to a SubGhzProtocolDecoderPowerSmart instance
 */
void* subghz_protocol_decoder_power_smart_alloc(SubGhzEnvironment* environment);

/**
 * Reset decoder SubGhzProtocolDecoderPowerSmart.
 * @param context Pointer to a SubGhzProtocolDecoderPowerSmart instance
 */
void subghz_protocol_decoder_power_smart_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderPowerSmart instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_power_smart_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderPowerSmart.
 * @param context Pointer to a SubGhzProtocolDecoderPowerSmart instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_power_smart_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderPowerSmart instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_power_smart_get_string(void* context, FuriString* output);
