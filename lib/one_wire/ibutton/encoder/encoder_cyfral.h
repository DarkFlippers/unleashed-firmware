/**
 * @file encoder_cyfral.h
 * 
 * Cyfral pulse format encoder
 */

#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EncoderCyfral EncoderCyfral;

/**
 * Allocate Cyfral encoder
 * @return EncoderCyfral* 
 */
EncoderCyfral* encoder_cyfral_alloc();

/**
 * Deallocate Cyfral encoder
 * @param cyfral 
 */
void encoder_cyfral_free(EncoderCyfral* cyfral);

/**
 * Reset Cyfral encoder
 * @param cyfral 
 */
void encoder_cyfral_reset(EncoderCyfral* cyfral);

/**
 * Set data to be encoded to Cyfral pulse format, 2 bytes
 * @param cyfral 
 * @param data 
 * @param data_size
 */
void encoder_cyfral_set_data(EncoderCyfral* cyfral, const uint8_t* data, size_t data_size);

/**
 * Pop pulse from Cyfral encoder
 * @param cyfral 
 * @param polarity 
 * @param length 
 */
void encoder_cyfral_get_pulse(EncoderCyfral* cyfral, bool* polarity, uint32_t* length);

#ifdef __cplusplus
}
#endif
