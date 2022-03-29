/**
 * @file pulse_decoder.h
 * 
 * Generic pulse protocol decoder library
 */

#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "pulse_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PulseDecoder PulseDecoder;

/**
 * Allocate decoder
 * @return PulseDecoder* 
 */
PulseDecoder* pulse_decoder_alloc();

/**
 * Deallocate decoder
 * @param decoder 
 */
void pulse_decoder_free(PulseDecoder* decoder);

/**
 * Add protocol to decoder
 * @param decoder 
 * @param protocol protocol implementation
 * @param index protocol index, should not be repeated
 */
void pulse_decoder_add_protocol(PulseDecoder* decoder, PulseProtocol* protocol, int32_t index);

/**
 * Push and process pulse with decoder
 * @param decoder 
 * @param polarity 
 * @param length 
 */
void pulse_decoder_process_pulse(PulseDecoder* decoder, bool polarity, uint32_t length);

/**
 * Get indec of decoded protocol
 * @param decoder 
 * @return int32_t, -1 if nothing decoded, or index of decoded protocol 
 */
int32_t pulse_decoder_get_decoded_index(PulseDecoder* decoder);

/**
 * Reset all protocols in decoder
 * @param decoder 
 */
void pulse_decoder_reset(PulseDecoder* decoder);

/**
 * Get decoded data from protocol
 * @param decoder 
 * @param index 
 * @param data 
 * @param length 
 */
void pulse_decoder_get_data(PulseDecoder* decoder, int32_t index, uint8_t* data, size_t length);

#ifdef __cplusplus
}
#endif