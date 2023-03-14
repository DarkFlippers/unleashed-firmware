#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_GT_WT_02_NAME "GT-WT02"

typedef struct WSProtocolDecoderGT_WT02 WSProtocolDecoderGT_WT02;
typedef struct WSProtocolEncoderGT_WT02 WSProtocolEncoderGT_WT02;

extern const SubGhzProtocolDecoder ws_protocol_gt_wt_02_decoder;
extern const SubGhzProtocolEncoder ws_protocol_gt_wt_02_encoder;
extern const SubGhzProtocol ws_protocol_gt_wt_02;

/**
 * Allocate WSProtocolDecoderGT_WT02.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderGT_WT02* pointer to a WSProtocolDecoderGT_WT02 instance
 */
void* ws_protocol_decoder_gt_wt_02_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderGT_WT02.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 */
void ws_protocol_decoder_gt_wt_02_free(void* context);

/**
 * Reset decoder WSProtocolDecoderGT_WT02.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 */
void ws_protocol_decoder_gt_wt_02_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_gt_wt_02_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_gt_wt_02_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderGT_WT02.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_gt_wt_02_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderGT_WT02.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_gt_wt_02_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderGT_WT02 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_gt_wt_02_get_string(void* context, FuriString* output);
