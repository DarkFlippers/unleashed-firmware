#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_GATE_TX_NAME "GateTX"

typedef struct SubGhzProtocolDecoderGateTx SubGhzProtocolDecoderGateTx;
typedef struct SubGhzProtocolEncoderGateTx SubGhzProtocolEncoderGateTx;

extern const SubGhzProtocolDecoder subghz_protocol_gate_tx_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_gate_tx_encoder;
extern const SubGhzProtocol subghz_protocol_gate_tx;

/**
 * Allocate SubGhzProtocolEncoderGateTx.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderGateTx* pointer to a SubGhzProtocolEncoderGateTx instance
 */
void* subghz_protocol_encoder_gate_tx_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderGateTx.
 * @param context Pointer to a SubGhzProtocolEncoderGateTx instance
 */
void subghz_protocol_encoder_gate_tx_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderGateTx instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_gate_tx_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderGateTx instance
 */
void subghz_protocol_encoder_gate_tx_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderGateTx instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_gate_tx_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderGateTx.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderGateTx* pointer to a SubGhzProtocolDecoderGateTx instance
 */
void* subghz_protocol_decoder_gate_tx_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderGateTx.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 */
void subghz_protocol_decoder_gate_tx_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderGateTx.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 */
void subghz_protocol_decoder_gate_tx_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_gate_tx_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_gate_tx_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderGateTx.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_decoder_gate_tx_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzPresetDefinition* preset);

/**
 * Deserialize data SubGhzProtocolDecoderGateTx.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_gate_tx_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderGateTx instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_gate_tx_get_string(void* context, string_t output);
