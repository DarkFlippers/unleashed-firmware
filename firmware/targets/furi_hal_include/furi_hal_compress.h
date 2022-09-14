/**
 * @file furi_hal_compress.h
 * LZSS based compression HAL API
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Defines encoder and decoder window size */
#define FURI_HAL_COMPRESS_EXP_BUFF_SIZE_LOG (8)

/** Defines encoder and decoder lookahead buffer size */
#define FURI_HAL_COMPRESS_LOOKAHEAD_BUFF_SIZE_LOG (4)

/** FuriHalCompress control structure */
typedef struct FuriHalCompress FuriHalCompress;

/** Initialize icon decoder
 */
void furi_hal_compress_icon_init();

/** Icon decoder
 *
 * @param   icon_data    pointer to icon data
 * @param   decoded_buff pointer to decoded buffer
 */
void furi_hal_compress_icon_decode(const uint8_t* icon_data, uint8_t** decoded_buff);

/** Allocate encoder and decoder
 *
 * @param   compress_buff_size  size of decoder and encoder buffer to allocate
 *
 * @return  FuriHalCompress instance
 */
FuriHalCompress* furi_hal_compress_alloc(uint16_t compress_buff_size);

/** Free encoder and decoder
 *
 * @param   compress  FuriHalCompress instance
 */
void furi_hal_compress_free(FuriHalCompress* compress);

/** Encode data
 *
 * @param   compress FuriHalCompress instance
 * @param   data_in pointer to input data
 * @param   data_in_size size of input data
 * @param   data_out maximum size of output data
 * @param   data_res_size pointer to result output data size
 *
 * @return  true on success
 */
bool furi_hal_compress_encode(
    FuriHalCompress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

/** Decode data
 *
 * @param   compress FuriHalCompress instance
 * @param   data_in pointer to input data
 * @param   data_in_size size of input data
 * @param   data_out maximum size of output data
 * @param   data_res_size pointer to result output data size
 *
 * @return  true on success
 */
bool furi_hal_compress_decode(
    FuriHalCompress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

#ifdef __cplusplus
}
#endif
