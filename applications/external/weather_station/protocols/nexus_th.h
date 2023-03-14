#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_NEXUS_TH_NAME "Nexus-TH"

typedef struct WSProtocolDecoderNexus_TH WSProtocolDecoderNexus_TH;
typedef struct WSProtocolEncoderNexus_TH WSProtocolEncoderNexus_TH;

extern const SubGhzProtocolDecoder ws_protocol_nexus_th_decoder;
extern const SubGhzProtocolEncoder ws_protocol_nexus_th_encoder;
extern const SubGhzProtocol ws_protocol_nexus_th;

/**
 * Allocate WSProtocolDecoderNexus_TH.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderNexus_TH* pointer to a WSProtocolDecoderNexus_TH instance
 */
void* ws_protocol_decoder_nexus_th_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderNexus_TH.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 */
void ws_protocol_decoder_nexus_th_free(void* context);

/**
 * Reset decoder WSProtocolDecoderNexus_TH.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 */
void ws_protocol_decoder_nexus_th_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_nexus_th_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_nexus_th_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderNexus_TH.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_nexus_th_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderNexus_TH.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_nexus_th_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderNexus_TH instance
 * @param output Resulting text
 */
void ws_protocol_decoder_nexus_th_get_string(void* context, FuriString* output);
