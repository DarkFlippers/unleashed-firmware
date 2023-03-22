/**
 * @file compress.h
 * LZSS based compression HAL API
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Compress Icon control structure */
typedef struct CompressIcon CompressIcon;

/** Initialize icon compressor
 *
 * @return     Compress Icon instance
 */
CompressIcon* compress_icon_alloc();

/** Free icon compressor
 *
 * @param      instance  The Compress Icon instance
 */
void compress_icon_free(CompressIcon* instance);

/** Decompress icon
 *
 * @warning    decoded_buff pointer set by this function is valid till next
 *             `compress_icon_decode` or `compress_icon_free` call
 *
 * @param      instance      The Compress Icon instance
 * @param      icon_data     pointer to icon data
 * @param[in]  decoded_buff  pointer to decoded buffer pointer
 */
void compress_icon_decode(CompressIcon* instance, const uint8_t* icon_data, uint8_t** decoded_buff);

/** Compress control structure */
typedef struct Compress Compress;

/** Allocate encoder and decoder
 *
 * @param   compress_buff_size  size of decoder and encoder buffer to allocate
 *
 * @return  Compress instance
 */
Compress* compress_alloc(uint16_t compress_buff_size);

/** Free encoder and decoder
 *
 * @param   compress  Compress instance
 */
void compress_free(Compress* compress);

/** Encode data
 *
 * @param   compress Compress instance
 * @param   data_in pointer to input data
 * @param   data_in_size size of input data
 * @param   data_out maximum size of output data
 * @param   data_res_size pointer to result output data size
 *
 * @return  true on success
 */
bool compress_encode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

/** Decode data
 *
 * @param   compress Compress instance
 * @param   data_in pointer to input data
 * @param   data_in_size size of input data
 * @param   data_out maximum size of output data
 * @param   data_res_size pointer to result output data size
 *
 * @return  true on success
 */
bool compress_decode(
    Compress* compress,
    uint8_t* data_in,
    size_t data_in_size,
    uint8_t* data_out,
    size_t data_out_size,
    size_t* data_res_size);

#ifdef __cplusplus
}
#endif
