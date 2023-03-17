#pragma once

#include <lib/subghz/protocols/base.h>

#include <lib/subghz/blocks/const.h>
#include <lib/subghz/blocks/decoder.h>
#include <lib/subghz/blocks/encoder.h>
#include "ws_generic.h"
#include <lib/subghz/blocks/math.h>

#define WS_PROTOCOL_GT_WT_03_NAME "GT-WT03"

typedef struct WSProtocolDecoderGT_WT03 WSProtocolDecoderGT_WT03;
typedef struct WSProtocolEncoderGT_WT03 WSProtocolEncoderGT_WT03;

extern const SubGhzProtocolDecoder ws_protocol_gt_wt_03_decoder;
extern const SubGhzProtocolEncoder ws_protocol_gt_wt_03_encoder;
extern const SubGhzProtocol ws_protocol_gt_wt_03;

/**
 * Allocate WSProtocolDecoderGT_WT03.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return WSProtocolDecoderGT_WT03* pointer to a WSProtocolDecoderGT_WT03 instance
 */
void* ws_protocol_decoder_gt_wt_03_alloc(SubGhzEnvironment* environment);

/**
 * Free WSProtocolDecoderGT_WT03.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 */
void ws_protocol_decoder_gt_wt_03_free(void* context);

/**
 * Reset decoder WSProtocolDecoderGT_WT03.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 */
void ws_protocol_decoder_gt_wt_03_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void ws_protocol_decoder_gt_wt_03_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 * @return hash Hash sum
 */
uint8_t ws_protocol_decoder_gt_wt_03_get_hash_data(void* context);

/**
 * Serialize data WSProtocolDecoderGT_WT03.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus ws_protocol_decoder_gt_wt_03_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data WSProtocolDecoderGT_WT03.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    ws_protocol_decoder_gt_wt_03_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a WSProtocolDecoderGT_WT03 instance
 * @param output Resulting text
 */
void ws_protocol_decoder_gt_wt_03_get_string(void* context, FuriString* output);
