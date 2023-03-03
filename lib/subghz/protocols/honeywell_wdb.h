#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_HONEYWELL_WDB_NAME "Honeywell"

typedef struct SubGhzProtocolDecoderHoneywell_WDB SubGhzProtocolDecoderHoneywell_WDB;
typedef struct SubGhzProtocolEncoderHoneywell_WDB SubGhzProtocolEncoderHoneywell_WDB;

extern const SubGhzProtocolDecoder subghz_protocol_honeywell_wdb_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_honeywell_wdb_encoder;
extern const SubGhzProtocol subghz_protocol_honeywell_wdb;

/**
 * Allocate SubGhzProtocolEncoderHoneywell_WDB.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderHoneywell_WDB* pointer to a SubGhzProtocolEncoderHoneywell_WDB instance
 */
void* subghz_protocol_encoder_honeywell_wdb_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderHoneywell_WDB.
 * @param context Pointer to a SubGhzProtocolEncoderHoneywell_WDB instance
 */
void subghz_protocol_encoder_honeywell_wdb_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderHoneywell_WDB instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_honeywell_wdb_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderHoneywell_WDB instance
 */
void subghz_protocol_encoder_honeywell_wdb_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderHoneywell_WDB instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_honeywell_wdb_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderHoneywell_WDB.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderHoneywell_WDB* pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 */
void* subghz_protocol_decoder_honeywell_wdb_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderHoneywell_WDB.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 */
void subghz_protocol_decoder_honeywell_wdb_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderHoneywell_WDB.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 */
void subghz_protocol_decoder_honeywell_wdb_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_honeywell_wdb_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_honeywell_wdb_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderHoneywell_WDB.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_honeywell_wdb_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderHoneywell_WDB.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_honeywell_wdb_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderHoneywell_WDB instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_honeywell_wdb_get_string(void* context, FuriString* output);
