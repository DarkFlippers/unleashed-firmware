#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_RAW_NAME "RAW"

typedef void (*SubGhzProtocolEncoderRAWCallbackEnd)(void* context);

typedef struct SubGhzProtocolDecoderRAW SubGhzProtocolDecoderRAW;
typedef struct SubGhzProtocolEncoderRAW SubGhzProtocolEncoderRAW;

extern const SubGhzProtocolDecoder subghz_protocol_raw_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_raw_encoder;
extern const SubGhzProtocol subghz_protocol_raw;

/**
 * Open file for writing
 * @param instance Pointer to a SubGhzProtocolDecoderRAW instance
 * @param dev_name  File name
 * @param preset The modulation on which the signal was received, SubGhzPresetDefinition
 * @return true On success
 */
bool subghz_protocol_raw_save_to_file_init(
    SubGhzProtocolDecoderRAW* instance,
    const char* dev_name,
    SubGhzPresetDefinition* preset);

/**
 * Stop writing file to flash
 * @param instance Pointer to a SubGhzProtocolDecoderRAW instance
 */
void subghz_protocol_raw_save_to_file_stop(SubGhzProtocolDecoderRAW* instance);

/**
 * Get the number of samples received SubGhzProtocolDecoderRAW.
 * @param instance Pointer to a SubGhzProtocolDecoderRAW instance
 * @return count of samples
 */
size_t subghz_protocol_raw_get_sample_write(SubGhzProtocolDecoderRAW* instance);

/**
 * Allocate SubGhzProtocolDecoderRAW.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderRAW* pointer to a SubGhzProtocolDecoderRAW instance
 */
void* subghz_protocol_decoder_raw_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderRAW.
 * @param context Pointer to a SubGhzProtocolDecoderRAW instance
 */
void subghz_protocol_decoder_raw_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderRAW.
 * @param context Pointer to a SubGhzProtocolDecoderRAW instance
 */
void subghz_protocol_decoder_raw_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderRAW instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_raw_feed(void* context, bool level, uint32_t duration);

/**
 * Deserialize data SubGhzProtocolDecoderRAW.
 * @param context Pointer to a SubGhzProtocolDecoderRAW instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_decoder_raw_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderRAW instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_raw_get_string(void* context, string_t output);

/**
 * Allocate SubGhzProtocolEncoderRAW.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderRAW* pointer to a SubGhzProtocolEncoderRAW instance
 */
void* subghz_protocol_encoder_raw_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderRAW.
 * @param context Pointer to a SubGhzProtocolEncoderRAW instance
 */
void subghz_protocol_encoder_raw_free(void* context);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderRAW instance
 */
void subghz_protocol_encoder_raw_stop(void* context);

/**
 * Сallback on completion of file transfer.
 * @param context Pointer to a SubGhzProtocolEncoderRAW instance
 */
void subghz_protocol_raw_file_encoder_worker_callback_end(void* context);

/**
 * Set callback on completion of file transfer.
 * @param instance Pointer to a SubGhzProtocolEncoderRAW instance
 * @param callback_end Callback, SubGhzProtocolEncoderRAWCallbackEnd
 * @param context_end Context
 */
void subghz_protocol_raw_file_encoder_worker_set_callback_end(
    SubGhzProtocolEncoderRAW* instance,
    SubGhzProtocolEncoderRAWCallbackEnd callback_end,
    void* context_end);

/**
 * File generation for RAW work.
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param file_path File path
 */
void subghz_protocol_raw_gen_fff_data(FlipperFormat* flipper_format, const char* file_path);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderRAW instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return true On success
 */
bool subghz_protocol_encoder_raw_deserialize(void* context, FlipperFormat* flipper_format);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderRAW instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_raw_yield(void* context);
