/**
 * @file encoder_metakom.h
 * 
 * Metakom pulse format encoder
 */

#pragma once
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct EncoderMetakom EncoderMetakom;

/**
 * Allocate Metakom encoder
 * @return EncoderMetakom* 
 */
EncoderMetakom* encoder_metakom_alloc();

/**
 * Deallocate Metakom encoder
 * @param metakom 
 */
void encoder_metakom_free(EncoderMetakom* metakom);

/**
 * Reset Metakom encoder
 * @param metakom 
 */
void encoder_metakom_reset(EncoderMetakom* metakom);

/**
 * Set data to be encoded to Metakom pulse format, 4 bytes
 * @param metakom 
 * @param data 
 * @param data_size 
 */
void encoder_metakom_set_data(EncoderMetakom* metakom, const uint8_t* data, size_t data_size);

/**
 * Pop pulse from Metakom encoder
 * @param cyfral 
 * @param polarity 
 * @param length 
 */
void encoder_metakom_get_pulse(EncoderMetakom* metakom, bool* polarity, uint32_t* length);

#ifdef __cplusplus
}
#endif