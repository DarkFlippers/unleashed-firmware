#pragma once

#include "base.h"

#define SUBGHZ_PROTOCOL_DICKERT_MAHS_NAME "Dickert_MAHS"

typedef struct SubGhzProtocolDecoderDickertMAHS SubGhzProtocolDecoderDickertMAHS;
typedef struct SubGhzProtocolEncoderDickertMAHS SubGhzProtocolEncoderDickertMAHS;

extern const SubGhzProtocolDecoder subghz_protocol_dickert_mahs_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_dickert_mahs_encoder;
extern const SubGhzProtocol subghz_protocol_dickert_mahs;

/** Allocate SubGhzProtocolEncoderDickertMAHS.
 *
 * @param      environment  Pointer to a SubGhzEnvironment instance
 *
 * @return     pointer to a SubGhzProtocolEncoderDickertMAHS instance
 */
void* subghz_protocol_encoder_dickert_mahs_alloc(SubGhzEnvironment* environment);

/** Free SubGhzProtocolEncoderDickertMAHS.
 *
 * @param      context  Pointer to a SubGhzProtocolEncoderDickertMAHS instance
 */
void subghz_protocol_encoder_dickert_mahs_free(void* context);

/** Deserialize and generating an upload to send.
 *
 * @param      context         Pointer to a SubGhzProtocolEncoderDickertMAHS
 *                             instance
 * @param      flipper_format  Pointer to a FlipperFormat instance
 *
 * @return     status
 */
SubGhzProtocolStatus
    subghz_protocol_encoder_dickert_mahs_deserialize(void* context, FlipperFormat* flipper_format);

/** Forced transmission stop.
 *
 * @param      context  Pointer to a SubGhzProtocolEncoderDickertMAHS instance
 */
void subghz_protocol_encoder_dickert_mahs_stop(void* context);

/** Getting the level and duration of the upload to be loaded into DMA.
 *
 * @param      context  Pointer to a SubGhzProtocolEncoderDickertMAHS instance
 *
 * @return     LevelDuration
 */
LevelDuration subghz_protocol_encoder_dickert_mahs_yield(void* context);

/** Allocate SubGhzProtocolDecoderDickertMAHS.
 *
 * @param      environment  Pointer to a SubGhzEnvironment instance
 *
 * @return     pointer to a SubGhzProtocolDecoderDickertMAHS instance
 */
void* subghz_protocol_decoder_dickert_mahs_alloc(SubGhzEnvironment* environment);

/** Free SubGhzProtocolDecoderDickertMAHS.
 *
 * @param      context  Pointer to a SubGhzProtocolDecoderDickertMAHS instance
 */
void subghz_protocol_decoder_dickert_mahs_free(void* context);

/** Reset decoder SubGhzProtocolDecoderDickertMAHS.
 *
 * @param      context  Pointer to a SubGhzProtocolDecoderDickertMAHS instance
 */
void subghz_protocol_decoder_dickert_mahs_reset(void* context);

/** Parse a raw sequence of levels and durations received from the air.
 *
 * @param      context   Pointer to a SubGhzProtocolDecoderDickertMAHS instance
 * @param      level     Signal level true-high false-low
 * @param      duration  Duration of this level in, us
 */
void subghz_protocol_decoder_dickert_mahs_feed(void* context, bool level, uint32_t duration);

/** Getting the hash sum of the last randomly received parcel.
 *
 * @param      context  Pointer to a SubGhzProtocolDecoderDickertMAHS instance
 *
 * @return     hash Hash sum
 */
uint8_t subghz_protocol_decoder_dickert_mahs_get_hash_data(void* context);

/** Serialize data SubGhzProtocolDecoderDickertMAHS.
 *
 * @param      context         Pointer to a SubGhzProtocolDecoderDickertMAHS
 *                             instance
 * @param      flipper_format  Pointer to a FlipperFormat instance
 * @param      preset          The modulation on which the signal was received,
 *                             SubGhzRadioPreset
 *
 * @return     status
 */
SubGhzProtocolStatus subghz_protocol_decoder_dickert_mahs_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);

/** Deserialize data SubGhzProtocolDecoderDickertMAHS.
 *
 * @param      context         Pointer to a SubGhzProtocolDecoderDickertMAHS
 *                             instance
 * @param      flipper_format  Pointer to a FlipperFormat instance
 *
 * @return     status
 */
SubGhzProtocolStatus
    subghz_protocol_decoder_dickert_mahs_deserialize(void* context, FlipperFormat* flipper_format);

/** Getting a textual representation of the received data.
 *
 * @param      context  Pointer to a SubGhzProtocolDecoderDickertMAHS instance
 * @param      output   Resulting text
 */
void subghz_protocol_decoder_dickert_mahs_get_string(void* context, FuriString* output);
