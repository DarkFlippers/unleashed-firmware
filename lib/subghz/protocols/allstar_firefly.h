#pragma once

/**
 * allstar_firefly.h  -  Allstar Firefly 318ALD31K native SubGHz protocol
 *
 * Registers the Allstar Firefly gate remote as a first-class SubGHz protocol,
 * supporting decode from air, save/load of .sub files, and retransmit.
 *
 * Protocol summary (measured from captured remotes):
 *   Carrier   : 318 MHz OOK  (FuriHalSubGhzPresetOok650Async)
 *   Code type : Static 9-bit trinary  (Supertex ED-9 encoder IC)
 *   Frame     : 18 symbols (2 per bit), no separate sync pulse
 *   Repeats   : 20 frames per keypress
 *   Inter-frame gap: ~30 440 us
 *
 *   Symbol encoding - each bit = two (pulse, gap) pairs:
 *     '+'  ON    : H H  = [4045us HIGH, 607us LOW] x2
 *     '-'  OFF   : L L  = [530us HIGH, 4139us LOW] x2
 *     '0'  FLOAT : H L  = [4045us HIGH, 607us LOW, 530us HIGH, 4139us LOW]
 *
 * Save file format:
 *   Filetype: Flipper SubGhz Key File
 *   Version: 1
 *   Frequency: 318000000
 *   Preset: FuriHalSubGhzPresetOok650Async
 *   Protocol: Allstar Firefly
 *   Key: +--000++-
 *
 * To register with the SubGHz app, in protocol_list.c add:
 *   #include "allstar_firefly.h"
 *   &subghz_protocol_allstar_firefly,   (in the protocol array)
 */

#include "base.h"

/* Protocol name (must match what is written to .sub files) */
#define SUBGHZ_PROTOCOL_ALLSTAR_FIREFLY_NAME "Allstar Firefly"

typedef struct SubGhzProtocolDecoderAllstarFirefly SubGhzProtocolDecoderAllstarFirefly;
typedef struct SubGhzProtocolEncoderAllstarFirefly SubGhzProtocolEncoderAllstarFirefly;

extern const SubGhzProtocolDecoder subghz_protocol_allstar_firefly_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_allstar_firefly_encoder;
extern const SubGhzProtocol subghz_protocol_allstar_firefly;

/**
 * Allocate SubGhzProtocolEncoderAllstarFirefly.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolEncoderAllstarFirefly* pointer to a SubGhzProtocolEncoderAllstarFirefly instance
 */
void* subghz_protocol_encoder_allstar_firefly_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolEncoderAllstarFirefly.
 * @param context Pointer to a SubGhzProtocolEncoderAllstarFirefly instance
 */
void subghz_protocol_encoder_allstar_firefly_free(void* context);

/**
 * Deserialize and generating an upload to send.
 * @param context Pointer to a SubGhzProtocolEncoderAllstarFirefly instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_encoder_allstar_firefly_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Forced transmission stop.
 * @param context Pointer to a SubGhzProtocolEncoderAllstarFirefly instance
 */
void subghz_protocol_encoder_allstar_firefly_stop(void* context);

/**
 * Getting the level and duration of the upload to be loaded into DMA.
 * @param context Pointer to a SubGhzProtocolEncoderAllstarFirefly instance
 * @return LevelDuration 
 */
LevelDuration subghz_protocol_encoder_allstar_firefly_yield(void* context);

/**
 * Allocate SubGhzProtocolDecoderAllstarFirefly.
 * @param environment Pointer to a SubGhzEnvironment instance
 * @return SubGhzProtocolDecoderAllstarFirefly* pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 */
void* subghz_protocol_decoder_allstar_firefly_alloc(SubGhzEnvironment* environment);

/**
 * Free SubGhzProtocolDecoderAllstarFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 */
void subghz_protocol_decoder_allstar_firefly_free(void* context);

/**
 * Reset decoder SubGhzProtocolDecoderAllstarFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 */
void subghz_protocol_decoder_allstar_firefly_reset(void* context);

/**
 * Parse a raw sequence of levels and durations received from the air.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 * @param level Signal level true-high false-low
 * @param duration Duration of this level in, us
 */
void subghz_protocol_decoder_allstar_firefly_feed(void* context, bool level, uint32_t duration);

/**
 * Getting the hash sum of the last randomly received parcel.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 * @return hash Hash sum
 */
uint8_t subghz_protocol_decoder_allstar_firefly_get_hash_data(void* context);

/**
 * Serialize data SubGhzProtocolDecoderAllstarFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @param preset The modulation on which the signal was received, SubGhzRadioPreset
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_allstar_firefly_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/**
 * Deserialize data SubGhzProtocolDecoderAllstarFirefly.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 * @param flipper_format Pointer to a FlipperFormat instance
 * @return status
 */
SubGhzProtocolStatus subghz_protocol_decoder_allstar_firefly_deserialize(
    void* context,
    FlipperFormat* flipper_format);

/**
 * Getting a textual representation of the received data.
 * @param context Pointer to a SubGhzProtocolDecoderAllstarFirefly instance
 * @param output Resulting text
 */
void subghz_protocol_decoder_allstar_firefly_get_string(void* context, FuriString* output);
