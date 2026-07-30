#pragma once

#include "base.h"

/*
 * Static (fixed-code) Manchester remote from the Telcoma / Cardin EDGE family
 * (EDGE2 / EDGE4 / EDGE20, also sold compatible with TANGO/QUASAR/SLIM/SE).
 *
 * 433.92 MHz, OOK/AM, Manchester encoded.
 * Half-bit (TE) ~= 1270us, full bit ~= 2540us (~394 bit/s).
 * On the wire: 0xFF preamble + per-remote serial + a one-hot CHANNEL marker.
 *   - The gate channel decodes as 32 Manchester bits + 1 trailing HIGH stop
 *     half-bit (e.g. 0xFF309FC0).
 *   - Every other channel carries an extra one-hot marker and decodes as 33
 *     bits; we normalise to a canonical 32-bit key (key = raw >> 1), where
 *     key[23:3] = serial and key[2:0] = one-hot channel (gate=0, others 1/2/4).
 * Fixed code: identical value on every press of a given channel; not rolling.
 *
 * NAMING: this is the Telcoma/Cardin EDGE 433.92 fixed-code protocol. It is
 * NOT the existing Cardin S449 (FSK KeeLoq rolling) or S466 (27MHz PWM); those
 * are different products. No public bit-level spec was found; the model here
 * was reverse-engineered from clean captures (gate verified against real RF).
 * The remote's encoder is an Atmel ATtiny25V MCU running custom firmware (not
 * a fixed-function encoder IC), which is why the protocol is undocumented.
 */
#define SUBGHZ_PROTOCOL_TELCOMA_EDGE_NAME "Telcoma/Cardin EDGE"

typedef struct SubGhzProtocolDecoderTelcomaEdge SubGhzProtocolDecoderTelcomaEdge;
typedef struct SubGhzProtocolEncoderTelcomaEdge SubGhzProtocolEncoderTelcomaEdge;

extern const SubGhzProtocolDecoder subghz_protocol_telcoma_edge_decoder;
extern const SubGhzProtocolEncoder subghz_protocol_telcoma_edge_encoder;
extern const SubGhzProtocol subghz_protocol_telcoma_edge;

void* subghz_protocol_decoder_telcoma_edge_alloc(SubGhzEnvironment* environment);
void subghz_protocol_decoder_telcoma_edge_free(void* context);
void subghz_protocol_decoder_telcoma_edge_reset(void* context);
void subghz_protocol_decoder_telcoma_edge_feed(void* context, bool level, uint32_t duration);
uint8_t subghz_protocol_decoder_telcoma_edge_get_hash_data(void* context);
SubGhzProtocolStatus subghz_protocol_decoder_telcoma_edge_serialize(
    void* context,
    FlipperFormat* flipper_format,
    SubGhzRadioPreset* preset);
SubGhzProtocolStatus
    subghz_protocol_decoder_telcoma_edge_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_decoder_telcoma_edge_get_string(void* context, FuriString* output);

void* subghz_protocol_encoder_telcoma_edge_alloc(SubGhzEnvironment* environment);
void subghz_protocol_encoder_telcoma_edge_free(void* context);
SubGhzProtocolStatus
    subghz_protocol_encoder_telcoma_edge_deserialize(void* context, FlipperFormat* flipper_format);
void subghz_protocol_encoder_telcoma_edge_stop(void* context);
LevelDuration subghz_protocol_encoder_telcoma_edge_yield(void* context);
