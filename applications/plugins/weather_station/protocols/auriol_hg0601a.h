#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_AURIOL_TH_NAME "Auriol HG06061" //HG06061A-DCF-TX

typedef struct WSProtocolDecoderAuriol_TH WSProtocolDecoderAuriol_TH;
typedef struct WSProtocolEncoderAuriol_TH WSProtocolEncoderAuriol_TH;

extern const SubGhzProtocolDecoder ws_protocol_auriol_th_decoder;
extern const SubGhzProtocolEncoder ws_protocol_auriol_th_encoder;
extern const SubGhzProtocol ws_protocol_auriol_th;

/**
 * Allocate WSProtocolDecoderAuriol_TH.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderAuriol_TH* pointer to a WSProtocolDecoderAuriol_TH instance
 */
void* ws_protocol_decoder_auriol_th_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderAuriol_TH.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 */
void ws_protocol_decoder_auriol_th_free(void* context);

/**
 * Reset decoder WSProtocolDecoderAuriol_TH.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 */
void ws_protocol_decoder_auriol_th_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_auriol_th_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_auriol_th_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderAuriol_TH.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_auriol_th_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderAuriol_TH.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_auriol_th_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderAuriol_TH instance
 * @param output Resulting text
 */
void ws_protocol_decoder_auriol_th_get_string(void* context, FuriString* output);
